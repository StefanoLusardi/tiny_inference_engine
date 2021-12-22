#include <functional>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

// TODO: check includes
#include <grpcpp/grpcpp.h>
#include <services.grpc.pb.h>

// TODO: rewrite proto using a new namespace and new messages
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

struct AsyncClientCall
{
	grpc::ClientContext context;
	grpc::Status result_code;
	virtual void proceed(bool ok) = 0;
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

	void proceed(bool ok) override
	{
		if (ok)
		{
			if (this->result_code.ok())
			{
				std::cout << this->response.message() << std::endl;
				if (this->_on_response_callback) 
					this->_on_response_callback(this->response);
			}
			else
			{
				std::cout << "RPC failed: (" << this->result_code.error_code() << ") " << this->result_code.error_message() << std::endl;
			}
		}

		delete this;
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
	std::unique_ptr<grpc::ClientAsyncReaderWriter<HelloRequest, ResponseT>> rpc;

	void proceed(bool ok) override
	{
		switch (this->call_state)
		{
		case CallState::CREATE:
			std::cout << "CREATE" << std::endl;
			break;
		
		case CallState::WRITE:
			std::cout << "WRITE"  << std::endl;
			break;

		case CallState::READ:
			std::cout << "READ" << std::endl;
			if (this->_on_response_callback) 
				this->_on_response_callback(this->response);

			this->call_state = CallState::DONE;
			rpc->WritesDone((void *)this);
			break;

		case CallState::DONE:
			std::cout << "DONE" << std::endl;
			this->call_state = CallState::FINISH;
			rpc->Finish(&this->result_code, (void *)this);
			break;

		case CallState::FINISH:
			std::cout << "FINISH" << std::endl;
			if(!this->result_code.ok()) 
				std::cout << "RPC failed: (" << this->result_code.error_code() << ") " << this->result_code.error_message() << std::endl;
	
			delete this;
			break;

		default:
			std::cerr << "Unexpected tag " << (int)this->call_state << std::endl;
			// TODO: static assert
			assert(false);
		}
	}
};

class AsyncClient final
{
public:
	explicit AsyncClient(const std::string &channel_address)
		: _stub{Greeter::NewStub(grpc::CreateChannel(channel_address, grpc::InsecureChannelCredentials()))}
		, _is_bidi_call_started{false}
	{
		_bidi_completion_queue = std::make_unique<grpc::CompletionQueue>();
		_bidi_grpc_thread = std::thread([this](const std::shared_ptr<grpc::CompletionQueue>& cq){ grpc_thread_worker(cq); }, _bidi_completion_queue);

		_async_completion_queue = std::make_unique<grpc::CompletionQueue>();
		for (auto thread_idx = 0; thread_idx < num_async_threads; ++thread_idx)
			_async_grpc_threads.emplace_back(std::thread([this](const std::shared_ptr<grpc::CompletionQueue>& cq){ grpc_thread_worker(cq); }, _async_completion_queue));
	}

	~AsyncClient()
	{
		std::cout << "Shutting down client" << std::endl;
		
		_bidi_completion_queue->Shutdown();		
		if(_bidi_grpc_thread.joinable())
			_bidi_grpc_thread.join();
		
		_async_completion_queue->Shutdown();
		for (auto&& t : _async_grpc_threads)
			if (t.joinable())
				t.join();
	}

	void start()
	{
		// // if(_bidi_call->call_state != AsyncClientBidiCall<HelloReply>::CallState::FINISH)
		// {
		// 	std::cout << "Unable to create a new Bidi Stream since one is already started." << std:: endl;
		// 	std::cout << "Call stop() to finish current Bidi Stream before creating a new one." << std:: endl;
		// 	return;
		// }

		_is_bidi_call_started = true;

		_bidi_call = new AsyncClientBidiCall<HelloReply>();
		_bidi_call->call_state = AsyncClientBidiCall<HelloReply>::CallState::CREATE;
		_bidi_call->set_response_callback([this](HelloReply&& response){ result_callback(response); });
		_bidi_call->rpc = _stub->PrepareAsyncSayHelloStream(&_bidi_call->context, _bidi_completion_queue.get());
		_bidi_call->rpc->StartCall((void *)_bidi_call);
	}

	void stop()
	{
		_bidi_call->call_state = AsyncClientBidiCall<HelloReply>::CallState::DONE;
		_bidi_call->rpc->WritesDone((void *)_bidi_call);
		_is_bidi_call_started = false;
	}

	void result_callback(HelloReply response)
	{
		std::cout << "result_callback:" << response.message() << std::endl;
	}

	void write_message(const std::string &msg, bool is_last = false)
	{
		_bidi_call->call_state = AsyncClientBidiCall<HelloReply>::CallState::WRITE;
		HelloRequest request;
		request.set_name(msg);
		
		if(is_last)
			_bidi_call->rpc->Write(request, grpc::WriteOptions().clear_buffer_hint(), (void *)_bidi_call);
		else
			_bidi_call->rpc->Write(request, grpc::WriteOptions().set_buffer_hint(), (void *)_bidi_call);
	}

	void read_message()
	{
		_bidi_call->call_state = AsyncClientBidiCall<HelloReply>::CallState::READ;
		_bidi_call->rpc->Read(&_bidi_call->response, (void *)_bidi_call);
	}

	void unary_sync()
	{
		std::cout << "Unary Sync call" << std::endl;

		HelloRequest request;
		request.set_name("Sync");

		HelloReply reply;

		grpc::Status status;
		grpc::ClientContext context;
		status = _stub->SayHello(&context, request, &reply);

		if (status.ok())
			std::cout << reply.message() << std::endl;
		else
			std::cout << "RPC failed: (" << status.error_code() << ") " << status.error_message() << std::endl;
	}

	void unary_async()
	{
		std::cout << "Unary Async call" << std::endl;

		HelloRequest request;
		request.set_name("Async");

		auto call = new AsyncClientUnaryCall<HelloReply>();
		call->rpc = _stub->PrepareAsyncSayHello(&call->context, request, _async_completion_queue.get());
		call->rpc->StartCall();
		call->rpc->Finish(&call->response, &call->result_code, (void *)call);
	}

private:
	void grpc_thread_worker(const std::shared_ptr<grpc::CompletionQueue>& cq)
	{
		while (true)
		{
			void *call_tag;
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
	
	std::unique_ptr<Greeter::Stub> _stub;
	std::vector<std::thread> _async_grpc_threads;
	std::shared_ptr<grpc::CompletionQueue> _async_completion_queue;
	std::thread _bidi_grpc_thread;
	std::shared_ptr<grpc::CompletionQueue> _bidi_completion_queue;
	AsyncClientBidiCall<HelloReply>* _bidi_call;

	bool _is_bidi_call_started;

	// TODO: Enable configuration 
	unsigned num_async_completion_queues = 2u;
	unsigned num_async_threads = 4u;
};

int main(int argc, char **argv)
{
	AsyncClient client("localhost:50051");

	std::string text;
	while (true)
	{
		std::cin >> text;
		if (text == "x")
		{
			break;
		}

		if (text == "start")
		{
			client.start();
			continue;
		}

		if (text == "stop")
		{
			client.stop();
			continue;
		}

		if (text == "s")
		{
			client.unary_sync();
			continue;
		}

		if (text == "a")
		{
			client.unary_async();
			continue;
		}

		if (text == "r")
		{
			client.read_message();
			continue;
		}

		if (text == "last")
			client.write_message(text, true);
		else
			client.write_message(text);
	}

	return 0;
}
