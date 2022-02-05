#include "grpc_server.hpp"
#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

namespace tie::server
{
class AsyncServerCall
{
public:
    AsyncServerCall(const std::shared_ptr<tie::InferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine_ptr)
        : _call_state{ CallState::CREATE }
        , _service{ service }
        , _cq{ cq }
        , _engine_ptr{ engine_ptr }
    {
    }

    virtual bool process(bool ok) = 0;

protected:
    enum class CallState : int
    {
        CREATE,
        PROCESS,
        FINISH
    };

    CallState _call_state;
    std::shared_ptr<tie::InferenceService::AsyncService> _service;
    std::shared_ptr<grpc::ServerCompletionQueue> _cq;
    std::shared_ptr<engine::engine_interface> _engine_ptr;
    std::mutex _mutex;
};

template<class RequestT, class ResponseT>
struct AsyncServerCallIO
{
    explicit AsyncServerCallIO()
        : rpc{ &context }
    {
    }
    RequestT request;
    ResponseT response;
    grpc::ServerContext context;
    grpc::ServerAsyncResponseWriter<ResponseT> rpc;
};

class EngineReadyAsyncCall : public AsyncServerCall
{
public:
    explicit EngineReadyAsyncCall(const std::shared_ptr<tie::InferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine_ptr)
        : AsyncServerCall(service, cq, engine_ptr)
        , _io{ AsyncServerCallIO<tie::EngineReadyRequest, tie::EngineReadyResponse>() }
    {
        spdlog::trace("creating EngineReadyAsyncCall");
        spdlog::debug("EngineReadyAsyncCall - CREATE");

        _service->RequestEngineReady(&_io.context, &_io.request, &_io.rpc, _cq.get(), _cq.get(), (void*)this);
        _call_state = AsyncServerCall::CallState::PROCESS;
    }

    ~EngineReadyAsyncCall() { spdlog::trace("deleting EngineReadyAsyncCall"); }

    bool process(bool ok) override
    {
        if (!ok)
        {
            // _call_state = CallState::FINISH;
            // _rpc.Finish(this->_reply, grpc::Status::CANCELLED, (void *)this);
            delete this;
            return false;
        }

        std::unique_lock<std::mutex> lock(_mutex);

        switch (_call_state)
        {
            case CallState::PROCESS:
            {
                spdlog::debug("EngineReadyAsyncCall - PROCESS");
                new EngineReadyAsyncCall(this->_service, this->_cq, this->_engine_ptr);

                const bool is_engine_ready = this->_engine_ptr->is_ready();
                _io.response.set_is_ready(is_engine_ready);
                _io.response.set_error_message("");

                spdlog::trace("EngineReadyAsyncCall - PROCESS - Is Engine Ready: {}", is_engine_ready);
                _call_state = CallState::FINISH;
                _io.rpc.Finish(_io.response, grpc::Status::OK, (void*)this);
                break;
            }

            case CallState::FINISH:
                spdlog::debug("EngineReadyAsyncCall - FINISH");
                lock.unlock();
                delete this;
                break;

            default: spdlog::error("EngineReadyAsyncCall - Unexpected tag: {}", int(_call_state)); assert(false);
        }

        return true;
    }

private:
    AsyncServerCallIO<tie::EngineReadyRequest, tie::EngineReadyResponse> _io;
};

class ModelReadyAsyncCall : public AsyncServerCall
{
public:
    explicit ModelReadyAsyncCall(const std::shared_ptr<tie::InferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine_ptr)
        : AsyncServerCall(service, cq, engine_ptr)
        , _io{ AsyncServerCallIO<tie::ModelReadyRequest, tie::ModelReadyResponse>() }
    {
        spdlog::trace("creating ModelReadyAsyncCall");
        spdlog::debug("ModelReadyAsyncCall - CREATE");

        _service->RequestModelReady(&_io.context, &_io.request, &_io.rpc, _cq.get(), _cq.get(), (void*)this);
        _call_state = AsyncServerCall::CallState::PROCESS;
    }

    ~ModelReadyAsyncCall() { spdlog::trace("deleting ModelReadyAsyncCall"); }

    bool process(bool ok) override
    {
        if (!ok)
        {
            // _call_state = CallState::FINISH;
            // _rpc.Finish(this->_reply, grpc::Status::CANCELLED, (void *)this);
            delete this;
            return false;
        }

        std::unique_lock<std::mutex> lock(_mutex);

        switch (_call_state)
        {
            case CallState::PROCESS:
            {
                spdlog::debug("ModelReadyAsyncCall - PROCESS");
                new ModelReadyAsyncCall(_service, _cq, _engine_ptr);

                const bool is_model_ready = _engine_ptr->is_model_ready(_io.request.model_name());
                _io.response.set_is_ready(is_model_ready);
                _io.response.set_error_message("");

                spdlog::trace("ModelReadyAsyncCall - PROCESS - Model Name: {} - Is Model Ready: {}", _io.request.model_name(), is_model_ready);
                _call_state = CallState::FINISH;
                _io.rpc.Finish(_io.response, grpc::Status::OK, (void*)this);
                break;
            }

            case CallState::FINISH:
                spdlog::debug("ModelReadyAsyncCall - FINISH");
                lock.unlock();
                delete this;
                break;

            default: spdlog::error("ModelReadyAsyncCall - Unexpected tag: {}", int(_call_state)); assert(false);
        }

        return true;
    }

private:
    AsyncServerCallIO<tie::ModelReadyRequest, tie::ModelReadyResponse> _io;
};

class LoadModelAsyncCall : public AsyncServerCall
{
public:
    explicit LoadModelAsyncCall(const std::shared_ptr<tie::InferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine_ptr)
        : AsyncServerCall(service, cq, engine_ptr)
        , _io{ AsyncServerCallIO<tie::ModelLoadRequest, tie::ModelLoadResponse>() }
    {
        spdlog::trace("creating LoadModelAsyncCall");
        spdlog::debug("LoadModelAsyncCall - CREATE");

        _service->RequestLoadModel(&_io.context, &_io.request, &_io.rpc, _cq.get(), _cq.get(), (void*)this);
        _call_state = AsyncServerCall::CallState::PROCESS;
    }

    ~LoadModelAsyncCall() { spdlog::trace("deleting LoadModelAsyncCall"); }

    bool process(bool ok) override
    {
        if (!ok)
        {
            // _call_state = CallState::FINISH;
            // _rpc.Finish(this->_reply, grpc::Status::CANCELLED, (void *)this);
            delete this;
            return false;
        }

        std::unique_lock<std::mutex> lock(_mutex);

        switch (_call_state)
        {
            case CallState::PROCESS:
            {
                spdlog::debug("LoadModelAsyncCall - PROCESS");
                new LoadModelAsyncCall(_service, _cq, _engine_ptr);

                const bool is_model_loaded = _engine_ptr->load_model(_io.request.model_name());
                _io.response.set_is_loaded(is_model_loaded);
                _io.response.set_error_message("");

                spdlog::trace("LoadModelAsyncCall - PROCESS - Model Loaded: {}", _io.request.model_name());
                _call_state = CallState::FINISH;
                _io.rpc.Finish(_io.response, grpc::Status::OK, (void*)this);
                break;
            }

            case CallState::FINISH:
                spdlog::debug("LoadModelAsyncCall - FINISH");
                lock.unlock();
                delete this;
                break;

            default: spdlog::error("LoadModelAsyncCall - Unexpected tag: {}", int(_call_state)); assert(false);
        }

        return true;
    }

private:
    AsyncServerCallIO<tie::ModelLoadRequest, tie::ModelLoadResponse> _io;
};

class UnloadModelAsyncCall : public AsyncServerCall
{
public:
    explicit UnloadModelAsyncCall(const std::shared_ptr<tie::InferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine_ptr)
        : AsyncServerCall(service, cq, engine_ptr)
        , _io{ AsyncServerCallIO<tie::UnloadModelRequest, tie::UnloadModelResponse>() }
    {
        spdlog::trace("creating UnloadModelAsyncCall");
        spdlog::debug("UnloadModelAsyncCall - CREATE");

        _service->RequestUnloadModel(&_io.context, &_io.request, &_io.rpc, _cq.get(), _cq.get(), (void*)this);
        _call_state = AsyncServerCall::CallState::PROCESS;
    }

    ~UnloadModelAsyncCall() { spdlog::trace("deleting UnloadModelAsyncCall"); }

    bool process(bool ok) override
    {
        if (!ok)
        {
            // _call_state = CallState::FINISH;
            // _rpc.Finish(this->_reply, grpc::Status::CANCELLED, (void *)this);
            delete this;
            return false;
        }

        std::unique_lock<std::mutex> lock(_mutex);

        switch (_call_state)
        {
            case CallState::PROCESS:
            {
                spdlog::debug("UnloadModelAsyncCall - PROCESS");
                new UnloadModelAsyncCall(_service, _cq, _engine_ptr);

                const bool is_model_unloaded = _engine_ptr->unload_model(_io.request.model_name());
                _io.response.set_is_unloaded(is_model_unloaded);
                _io.response.set_error_message("");

                spdlog::trace("UnloadModelAsyncCall - PROCESS - Model Unloaded: {}", _io.request.model_name());
                _call_state = CallState::FINISH;
                _io.rpc.Finish(_io.response, grpc::Status::OK, (void*)this);
                break;
            }

            case CallState::FINISH:
                spdlog::debug("UnloadModelAsyncCall - FINISH");
                lock.unlock();
                delete this;
                break;

            default: spdlog::error("UnloadModelAsyncCall - Unexpected tag: {}", int(_call_state)); assert(false);
        }

        return true;
    }

private:
    AsyncServerCallIO<tie::UnloadModelRequest, tie::UnloadModelResponse> _io;
};

class SingleInferenceCall : public AsyncServerCall
{
public:
    explicit SingleInferenceCall(const std::shared_ptr<tie::InferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine_ptr)
        : AsyncServerCall(service, cq, engine_ptr)
        , _io{ AsyncServerCallIO<tie::InferRequest, tie::InferResponse>() }
    {
        spdlog::trace("creating SingleInferenceCall");
        spdlog::debug("SingleInferenceCall - CREATE");

        _service->RequestInfer(&_io.context, &_io.request, &_io.rpc, _cq.get(), _cq.get(), (void*)this);
        _call_state = AsyncServerCall::CallState::PROCESS;
    }

    ~SingleInferenceCall() { spdlog::trace("deleting SingleInferenceCall"); }

    bool process(bool ok) override
    {
        if (!ok)
        {
            // _call_state = CallState::FINISH;
            // _rpc.Finish(this->_reply, grpc::Status::CANCELLED, (void *)this);
            delete this;
            return false;
        }

        std::unique_lock<std::mutex> lock(_mutex);

        switch (_call_state)
        {
            case CallState::PROCESS:
            {
                spdlog::debug("SingleInferenceCall - PROCESS");
                new SingleInferenceCall(_service, _cq, _engine_ptr);

                backend::infer_request request;
                // request.data = {_io.request.data().cbegin(), _io.request.data().cend()};
                request.model_name = _io.request.model_name();
                request.shape = { _io.request.shape().begin(), _io.request.shape().end() };
                spdlog::trace("SingleInferenceCall - PROCESS - Request - Model: {}", request.model_name);

                const backend::infer_response response = _engine_ptr->infer(request);

                _io.response.set_data(response.data.data());
                _io.response.set_error_message("");
                spdlog::trace("SingleInferenceCall - PROCESS - Response - Model: {}", request.model_name);

                _call_state = CallState::FINISH;
                _io.rpc.Finish(_io.response, grpc::Status::OK, (void*)this);
                break;
            }

            case CallState::FINISH:
                spdlog::debug("SingleInferenceCall - FINISH");
                lock.unlock();
                delete this;
                break;

            default: spdlog::error("SingleInferenceCall - Unexpected tag: {}", int(_call_state)); assert(false);
        }

        return true;
    }

private:
    AsyncServerCallIO<tie::InferRequest, tie::InferResponse> _io;
};

/*
template<class RequestT, class ResponseT>
class AsyncServerBidiCall : AsyncServerCallback<RequestT, ResponseT>
{
public:
	AsyncServerBidiCall(const std::shared_ptr<tie::InferenceService::AsyncService>& service, const std::shared_ptr<grpc::ServerCompletionQueue>& cq, const std::shared_ptr<engine::engine_interface>& engine_ptr)
		: AsyncServerCallback<RequestT, ResponseT>(service, cq, engine_ptr)
		, _rpc{&this->_context}
		, _call_state{CallState::CREATE}
	{
		this->_context.AsyncNotifyWhenDone((void *)this);
		this->_service->RequestSayHelloStream(&this->_context, &_rpc, this->_cq.get(), this->_cq.get(), (void *)this);
	}

	bool process(bool ok) override
	{
		std::unique_lock<std::mutex> lock(_mutex);

		switch (_call_state)
		{
		case CallState::READ:
		{
			if (!ok)
			{
				//Meaning client said it wants to end the stream either by a 'writedone' or 'finish' call.
				std::cout << "thread: " << std::this_thread::get_id() << " tag: " << this << " - READ" << " - Stop Process: ok=false" << std::endl;
				_call_state = CallState::DONE;
				_rpc.Finish(grpc::Status::OK, (void *)this);
				break;
			}

			std::cout << "thread: " << std::this_thread::get_id() << " tag: " << this << " - READ" << " - Request: " << this->_request.name() << std::endl;

			if (this->_request.name() == "last")
			{
				std::cout << "thread: " << std::this_thread::get_id() << " tag: " << this << " - Sending Server Response" << std::endl;
				this->_response.set_message("Server Reply");
				
				std::this_thread::sleep_for(3s);
				_call_state = CallState::WRITE;
				_rpc.Write(this->_response, (void *)this);
				break;
			}

			_call_state = CallState::READ;
			_rpc.Read(&this->_request, (void *)this);
			break;
		}

		case CallState::WRITE:
			std::cout << "thread: " << std::this_thread::get_id() << " tag: " << this << " - WRITE" << " - Reply: " << this->_response.message() << std::endl;
			_call_state = CallState::READ;
			_rpc.Read(&this->_request, (void *)this);
			break;

		case CallState::CREATE:
			if (!ok)
			{
				std::cout << "thread: " << std::this_thread::get_id() << " tag: " << this << " - CREATE" << " - Stop Process: ok=false" << std::endl;

				lock.unlock();
				delete this;
				return false;
			}

			std::cout << "thread: " << std::this_thread::get_id() << " tag: " << this << " - CREATE" << std::endl;
			new AsyncServerBidiCall<RequestT, ResponseT>(this->_service, this->_cq, this->_engine_ptr);
			_call_state = CallState::READ;
			_rpc.Read(&this->_request, (void *)this);
			break;

		case CallState::DONE:
			std::cout << "thread: " << std::this_thread::get_id() << " tag: " << this << " - DONE" << " - Context cancelled:" << std::boolalpha  << this->_context.IsCancelled() << std::endl;
			_call_state = CallState::FINISH;
			break;

		case CallState::FINISH:
			std::cout << "thread: " << std::this_thread::get_id() << " tag: " << this << " - FINISH" << " - Context cancelled:" << std::boolalpha  << this->_context.IsCancelled() << std::endl;
			lock.unlock();
			delete this;
			break;

		default:
			std::cerr << "Unexpected tag " << int(_call_state) << std::endl;
			assert(false);
		}

		return true;
	}

private:
	enum class CallState : int
	{
		READ = 1,
		WRITE = 2,
		CREATE = 3,
		DONE = 4,
		FINISH = 5
	};

	grpc::ServerAsyncReaderWriter<HelloReply, HelloRequest> _rpc;
	CallState _call_state;
	std::mutex _mutex;
};
*/

grpc_server::grpc_server(const std::shared_ptr<engine::engine_interface>& engine_ptr)
    : _engine_ptr{ engine_ptr }
    , _is_running{ false }
{
    spdlog::trace("creating grpc_server");

    const std::string address = "0.0.0.0";
    const std::string port = "50051";
    _server_uri = address + ":" + port;
    _num_threads_inference_multi = 1;
    _num_threads_inference_single = 1;

    // const std::string address = get_environment_variable("SERVER_ADDRESS", "0.0.0.0");
    // const std::string port = get_environment_variable("SERVER_PORT", "50051");
    // _server_uri = address + ":" + port;
    // _num_threads_bidi_call = std::stoi(get_environment_variable("NUM_THREADS_BIDI", "1"));
    // _num_threads_unary_call = std::stoi(get_environment_variable("NUM_THREADS_UNARY", "4"));
}

grpc_server::~grpc_server()
{
    spdlog::trace("deleting grpc_server");
    if (_is_running) stop();
}

void grpc_server::start()
{
    spdlog::info("start grpc_server");

    _service = std::make_shared<tie::InferenceService::AsyncService>();

    grpc::ServerBuilder builder;
    builder.AddListeningPort(_server_uri, grpc::InsecureServerCredentials());
    builder.RegisterService(_service.get());

    // builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 1000);
    // builder.SetDefaultCompressionLevel(GRPC_COMPRESS_LEVEL_HIGH);
    // builder.SetDefaultCompressionAlgorithm(GRPC_COMPRESS_GZIP);

    if (_num_threads_inference_multi > 0) _completion_queues.emplace("multi_inference_queue", builder.AddCompletionQueue());

    if (_num_threads_inference_single > 0) _completion_queues.emplace("single_inference_queue", builder.AddCompletionQueue());

    _completion_queues.emplace("engine_queries_queue", builder.AddCompletionQueue());

    _server = builder.BuildAndStart();
    _is_running = true;

    // Multi (Batch) Inferece Call
    for (auto thread_idx = 0; thread_idx < _num_threads_inference_multi; ++thread_idx)
    {
        const auto cq = _completion_queues.at("multi_inference_queue");
        //new BatchInferenceCall<tie::InferRequest, tie::InferResponse>(_service, cq, _engine_ptr);
        _server_threads.emplace_back([this](const auto& cq) { grpc_thread_worker(cq); }, cq);
    }

    // Single Inferece Call
    for (auto thread_idx = 0; thread_idx < _num_threads_inference_single; ++thread_idx)
    {
        const auto cq = _completion_queues.at("single_inference_queue");
        new SingleInferenceCall(_service, cq, _engine_ptr);
        _server_threads.emplace_back([this](const auto& cq) { grpc_thread_worker(cq); }, cq);
    }

    // Engine Queries Calls
    {
        const auto cq = _completion_queues.at("engine_queries_queue");
        new EngineReadyAsyncCall(_service, cq, _engine_ptr);
        new ModelReadyAsyncCall(_service, cq, _engine_ptr);
        new LoadModelAsyncCall(_service, cq, _engine_ptr);
        new UnloadModelAsyncCall(_service, cq, _engine_ptr);
        _server_threads.emplace_back([this](const auto& cq) { grpc_thread_worker(cq); }, cq);
    }

    spdlog::info("Server running @ {}", _server_uri);
    spdlog::info("Single Inference Threads: {}", _num_threads_inference_single);
    spdlog::info("Multi (Batch) Inference Threads: {}", _num_threads_inference_multi);
}

void grpc_server::stop()
{
    spdlog::info("stop grpc_server");

    _server->Shutdown();

    for (auto& t : _server_threads)
        if (t.joinable()) t.join();

    for (auto&& [_, cq] : _completion_queues) cq->Shutdown();

    // Drain Completion Queues
    void* drain_tag = nullptr;
    bool ok = false;
    for (auto&& [_, cq] : _completion_queues)
        while (cq->Next(&drain_tag, &ok))
            ;

    _is_running = false;
}

void grpc_server::grpc_thread_worker(const std::shared_ptr<grpc::ServerCompletionQueue>& cq)
{
    while (_is_running)
    {
        void* call_tag = nullptr;
        bool ok = false;
        if (!cq->Next(&call_tag, &ok))
        {
            spdlog::warn("Server stream closed. Finish worker thread");
            break;
        }

        auto call = static_cast<AsyncServerCall*>(call_tag);
        if (!call->process(ok)) break;
    }
}

}