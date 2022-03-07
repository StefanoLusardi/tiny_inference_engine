#include "grpc_client.hpp"

#include <iostream>

namespace tie::client_core
{

struct AsyncClientCall
{
    grpc::ClientContext context;
    grpc::Status result_code;
    virtual bool proceed(bool ok) = 0;
};

template<class ResponseT>
struct AsyncClientCallback : public AsyncClientCall
{
    ResponseT response;
    void set_response_callback(std::function<void(ResponseT)> response_callback) { _on_response_callback = response_callback; }

protected:
    std::function<void(ResponseT)> _on_response_callback;
};

template<class ResponseT>
struct AsyncClientUnaryCall : public AsyncClientCallback<ResponseT>
{
    std::unique_ptr<grpc::ClientAsyncResponseReader<ResponseT>> rpc;

    bool proceed(bool ok) override
    {
        if (ok)
        {
            if (this->result_code.ok())
            {
                // std::cout << this->response.message() << std::endl;
                if (this->_on_response_callback) this->_on_response_callback(this->response);
            }
            else
            {
                std::cout << "RPC failed: (" << this->result_code.error_code() << ") " << this->result_code.error_message() << std::endl;
            }
        }

        return false;
    }
};

template<class ResponseT>
struct AsyncClientBidiCall : public AsyncClientCallback<ResponseT>
{
    enum class CallState : int
    {
        READ = 1,
        WRITE = 2,
        CREATE = 3,
        DONE = 4,
        FINISH = 5
    };

    CallState call_state;
    std::unique_ptr<grpc::ClientAsyncReaderWriter<tie::InferRequest, tie::InferResponse>> rpc;

    bool proceed(bool ok) override
    {
        switch (this->call_state)
        {
            case CallState::CREATE: std::cout << "CREATE" << std::endl; break;

            case CallState::WRITE: std::cout << "WRITE" << std::endl; return true;

            case CallState::READ:
                std::cout << "READ" << std::endl;
                if (this->_on_response_callback) this->_on_response_callback(this->response);

                this->call_state = CallState::DONE;
                rpc->WritesDone((void*)this);
                return true;

            case CallState::DONE:
                std::cout << "DONE" << std::endl;
                this->call_state = CallState::FINISH;
                rpc->Finish(&this->result_code, (void*)this);
                return true;

            case CallState::FINISH:
                std::cout << "FINISH" << std::endl;
                if (!this->result_code.ok()) std::cout << "RPC failed: (" << this->result_code.error_code() << ") " << this->result_code.error_message() << std::endl;
                return false;

            default:
                std::cerr << "Unexpected tag " << (int)this->call_state << std::endl;
                // TODO: static assert
                assert(false);
                return false;
        }

        return false;
    }
};

grpc_client::grpc_client(const std::string& channel_address)
    : _stub{ tie::InferenceService::NewStub(grpc::CreateChannel(channel_address, grpc::InsecureChannelCredentials())) }
    , _is_bidi_stream_enabled{ false }
{
    if (_is_bidi_stream_enabled)
    {
        _bidi_completion_queue = std::make_unique<grpc::CompletionQueue>();
        _bidi_grpc_thread = std::thread([this](const std::shared_ptr<grpc::CompletionQueue>& cq) { grpc_thread_worker(cq); }, _bidi_completion_queue);
    }

    _async_completion_queue = std::make_unique<grpc::CompletionQueue>();
    for (auto thread_idx = 0; thread_idx < num_async_threads; ++thread_idx)
        _async_grpc_threads.emplace_back(std::thread([this](const std::shared_ptr<grpc::CompletionQueue>& cq) { grpc_thread_worker(cq); }, _async_completion_queue));
}

grpc_client::~grpc_client()
{
    std::cout << "Shutting down client" << std::endl;

    if (_is_bidi_stream_enabled)
    {
        _bidi_completion_queue->Shutdown();
        if (_bidi_grpc_thread.joinable()) _bidi_grpc_thread.join();
    }

    _async_completion_queue->Shutdown();
    for (auto&& t : _async_grpc_threads)
        if (t.joinable()) t.join();
}

/*

	void grpc_client::start_infer_stream()
	{
		if (_bidi_call && _bidi_call->call_state != AsyncClientBidiCall<tie::InferResponse>::CallState::FINISH)
		{
			std::cout << "Warning: bidi call must be in write state before call read." << std::endl;;
			return;
		}

		_bidi_call = new AsyncClientBidiCall<tie::InferResponse>();
		_bidi_call->call_state = AsyncClientBidiCall<tie::InferResponse>::CallState::CREATE;
		// _bidi_call->set_response_callback([this](tie::InferResponse&& response){ result_callback(response); });
		// _bidi_call->rpc = _stub->PrepareAsyncInferStream(&_bidi_call->context, _bidi_completion_queue.get());
		_bidi_call->rpc->StartCall((void *)_bidi_call);
	}

	void grpc_client::stop_infer_stream()
	{
		_bidi_call->call_state = AsyncClientBidiCall<tie::InferResponse>::CallState::DONE;
		_bidi_call->rpc->WritesDone((void *)_bidi_call);
	}

	void grpc_client::send_infer_stream_request(const std::string &msg, bool is_last)
	{
		if (_bidi_call->call_state != AsyncClientBidiCall<tie::InferResponse>::CallState::CREATE
		&& _bidi_call->call_state != AsyncClientBidiCall<tie::InferResponse>::CallState::WRITE)
		{
			std::cout << "Warning: bidi call must be in CREATE or WRITE state before call WRITE." << std::endl;;
			return;
		}

		_bidi_call->call_state = AsyncClientBidiCall<tie::InferResponse>::CallState::WRITE;
		tie::InferRequest request;
		// request.set_data(data);
		// request.set_model_name(model_name);
		// request.set_shape(shape);
		
		// TODO: check grpc::WriteOptions()
		_bidi_call->rpc->Write(request, (void *)_bidi_call);
	}

	void grpc_client::read_infer_stream_response()
	{
		if (_bidi_call->call_state != AsyncClientBidiCall<tie::InferResponse>::CallState::WRITE)
		{
			std::cout << "Warning: BIDI call must be in write state before call read." << std::endl;;
			return;
		}
		
		_bidi_call->call_state = AsyncClientBidiCall<tie::InferResponse>::CallState::READ;
		_bidi_call->rpc->Read(&_bidi_call->response, (void *)_bidi_call);
	}

*/

bool grpc_client::engine_ready_sync()
{
    std::cout << "engine_ready_sync" << std::endl;

    grpc::ClientContext context;
    tie::EngineReadyRequest request;
    tie::EngineReadyResponse response;
    grpc::Status status = _stub->EngineReady(&context, request, &response);

    if (status.ok())
    {
        std::cout << "engine is ready: " << response.is_ready() << std::endl;
    }
    else
    {
        std::cout << "RPC failed: (" << status.error_code() << ") " << status.error_message() << std::endl;
    }

    return response.is_ready();
}

void grpc_client::engine_ready_async(const std::function<void(bool)>& callback)
{
    std::cout << "engine_ready_async" << std::endl;

    tie::EngineReadyRequest request;

    auto call = new AsyncClientUnaryCall<tie::EngineReadyResponse>();
    call->set_response_callback([this](const tie::EngineReadyResponse& response) { _engine_ready_callback(response.is_ready()); });

    call->rpc = _stub->PrepareAsyncEngineReady(&call->context, request, _async_completion_queue.get());
    call->rpc->StartCall();
    call->rpc->Finish(&call->response, &call->result_code, (void*)call);
}

void grpc_client::engine_ready_async()
{
    std::cout << "engine_ready_async" << std::endl;

    tie::EngineReadyRequest request;

    auto call = new AsyncClientUnaryCall<tie::EngineReadyResponse>();
    call->set_response_callback([this](const tie::EngineReadyResponse& response) { _engine_ready_callback(response.is_ready()); });

    call->rpc = _stub->PrepareAsyncEngineReady(&call->context, request, _async_completion_queue.get());
    call->rpc->StartCall();
    call->rpc->Finish(&call->response, &call->result_code, (void*)call);
}

void grpc_client::set_engine_ready_callback(const std::function<void(bool)>& callback)
{
    _engine_ready_callback = callback;
    // std::cout << "engine_ready_async callback. engine is ready: " << response.is_ready() << std::endl;
}

bool grpc_client::model_ready_sync()
{
    std::cout << "model_ready_sync" << std::endl;

    tie::ModelReadyRequest request;
    request.set_model_name("some_model_name_from_input");

    tie::ModelReadyResponse response;

    grpc::Status status;
    grpc::ClientContext context;
    status = _stub->ModelReady(&context, request, &response);

    if (status.ok())
    {
        std::cout << "model: " << response.model_name() << " - ready: " << response.is_ready() << std::endl;
    }
    else
    {
        std::cout << "RPC failed: (" << status.error_code() << ") " << status.error_message() << std::endl;
    }

    return response.is_ready();
}

void grpc_client::model_ready_async()
{
    std::cout << "model_ready_async" << std::endl;

    tie::ModelReadyRequest request;
    request.set_model_name("some_model_name_from_input");

    auto call = new AsyncClientUnaryCall<tie::ModelReadyResponse>();
    call->rpc = _stub->PrepareAsyncModelReady(&call->context, request, _async_completion_queue.get());
    call->rpc->StartCall();
    call->rpc->Finish(&call->response, &call->result_code, (void*)call);
}

void grpc_client::set_model_ready_callback(const std::function<void(bool)>& callback)
{
    _model_ready_callback = callback;
    // std::cout << "model_ready_async callback. model: " << response.model_name() << ":" << response.is_ready() << std::endl;
}

void grpc_client::grpc_thread_worker(const std::shared_ptr<grpc::CompletionQueue>& cq)
{
    while (true)
    {
        void* call_tag;
        bool ok = false;

        if (!cq->Next(&call_tag, &ok))
        {
            std::cerr << "Client stream closed" << std::endl;
            break;
        }

        auto call = static_cast<AsyncClientCall*>(call_tag);
        call->proceed(ok);
    }
}

bool grpc_client::infer_sync(const tie::infer_request& infer_request)
{
    std::cout << "infer_sync" << std::endl;

    tie::InferRequest request;
    // request.set_data();
    // request.set_model();

    tie::InferResponse reply;
    grpc::ClientContext context;

    grpc::Status status = _stub->Infer(&context, request, &reply);
    if (status.ok()) std::cout << reply.error_message() << std::endl;
    else
        std::cout << "RPC failed: (" << status.error_code() << ") " << status.error_message() << std::endl;

    return true;
}

void grpc_client::infer_async()
{
    std::cout << "infer_async" << std::endl;

    // tie::InferRequest request;
    // request.set_name("Async");
    // auto call = new AsyncClientUnaryCall<tie::InferResponse>();
    // call->rpc = _stub->PrepareAsyncSayHello(&call->context, request, _async_completion_queue.get());
    // call->rpc->StartCall();
    // call->rpc->Finish(&call->response, &call->result_code, (void *)call);
}

void grpc_client::set_infer_callback(const std::function<void(bool)>& callback)
{
    // std::cout << "result_callback:" << response.message() << std::endl;
}

}
