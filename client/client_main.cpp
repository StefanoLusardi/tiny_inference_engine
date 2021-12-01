#include "services.pb.h"
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <condition_variable>

// TODO: check includes
#include <grpcpp/grpcpp.h>
#include <services.grpc.pb.h>

// TODO: remove
using grpc::ClientAsyncReaderWriter;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;

// TODO: rewrite proto using a new namespace and new messages
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

struct AsyncClientCall
{
	ClientContext context;
	Status result_code;
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


// TODO: move into AsyncClientBidiCall class
enum class Type : char
{
	READ = 1,
	WRITE = 2,
	CONNECT = 3,
	DONE = 4,
	FINISH = 5
};

template<class ResponseT>
struct AsyncClientBidiCall : public AsyncClientCallback<ResponseT>
{
	Type call_state;
	std::unique_ptr<grpc::ClientAsyncReaderWriter<HelloRequest, ResponseT>> rpc;

	void proceed(bool ok) override
	{
		switch (this->call_state)
		{
		case Type::CONNECT:
			std::cout << "CONNECT" << std::endl;
			break;
		
		case Type::WRITE:
			std::cout << "WRITE"  << std::endl;
			break;

		case Type::READ:
			std::cout << "READ" << std::endl;
			if (this->_on_response_callback) 
				this->_on_response_callback(this->response);

			this->call_state = Type::DONE;
			rpc->WritesDone((void *)this);
			break;

		case Type::DONE:
			std::cout << "DONE" << std::endl;
			this->call_state = Type::FINISH;
			rpc->Finish(&this->result_code, (void *)this);
			break;

		case Type::FINISH:
			std::cout << "FINISH" << std::endl;
			if(!this->result_code.ok()) std::cout << this->result_code.error_code() << this->result_code.error_message() << std::endl;
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
		: stub_{Greeter::NewStub(grpc::CreateChannel(channel_address, grpc::InsecureChannelCredentials()))},
		  grpc_thread_bidi_{std::make_unique<std::thread>([this] { grpc_thread_bidi(); })},
		  grpc_thread_unary_{std::make_unique<std::thread>([this] { grpc_thread_unary(); })}
	{
	}

	~AsyncClient()
	{
		std::cout << "Shutting down client" << std::endl;
		cq_.Shutdown();
		unary_cq_.Shutdown();

		if (grpc_thread_unary_->joinable())
			grpc_thread_unary_->join();

		if (grpc_thread_bidi_->joinable())
			grpc_thread_bidi_->join();
	}

	void start()
	{
		// TODO: use a class variable to check if "is_started"

		bidi_call = new AsyncClientBidiCall<HelloReply>();
		bidi_call->call_state = Type::CONNECT;
		bidi_call->set_response_callback([this](HelloReply&& response){ result_callback(response); });
		bidi_call->rpc = stub_->PrepareAsyncSayHelloStream(&bidi_call->context, &cq_);
		bidi_call->rpc->StartCall((void *)bidi_call);
	}

	void stop()
	{
		bidi_call->call_state = Type::DONE;
		bidi_call->rpc->WritesDone((void *)bidi_call);
	}

	void result_callback(HelloReply response)
	{
		std::cout << "result_callback:" << response.message() << std::endl;
	}

	void write_message(const std::string &msg)
	{
		bidi_call->call_state = Type::WRITE;
		HelloRequest request;
		request.set_name(msg);
		bidi_call->rpc->Write(request, (void *)bidi_call);
	}

	void read_message()
	{
		bidi_call->call_state = Type::READ;
		bidi_call->rpc->Read(&bidi_call->response, (void *)bidi_call);
	}

	void unary_sync()
	{
		std::cout << "Unary Sync call" << std::endl;

		HelloRequest request;
		request.set_name("Sync");

		HelloReply reply;

		Status status;
		ClientContext context;
		status = stub_->SayHello(&context, request, &reply);

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
		call->rpc = stub_->PrepareAsyncSayHello(&call->context, request, &unary_cq_);
		call->rpc->StartCall();
		call->rpc->Finish(&call->response, &call->result_code, (void *)call);
	}

private:
	// TODO: merge in a unique worker function "grpc_thread"
	void grpc_thread_bidi()
	{
		while (true)
		{
			void *call_tag;
			bool ok = false;

			if (!cq_.Next(&call_tag, &ok))
			{
				std::cerr << "Client stream closed" << std::endl;
				break;
			}

			AsyncClientCall* call = static_cast<AsyncClientCall*>(call_tag);
			call->proceed(ok);
		}
	}

	// TODO: merge in a unique worker function "grpc_thread"
	void grpc_thread_unary()
	{
		while (true)
		{
			void *call_tag;
			bool ok = false;

			if (!unary_cq_.Next(&call_tag, &ok))
			{
				std::cerr << "Client stream closed" << std::endl;
				break;
			}

			AsyncClientCall* call = static_cast<AsyncClientCall*>(call_tag);
			call->proceed(ok);
		}
	}

	// TODO: merge and use only only one for 2 threads
	CompletionQueue cq_;
	CompletionQueue unary_cq_;

	std::unique_ptr<std::thread> grpc_thread_bidi_;
	std::unique_ptr<std::thread> grpc_thread_unary_;
	
	std::unique_ptr<Greeter::Stub> stub_;
	AsyncClientBidiCall<HelloReply>* bidi_call;
};

int main(int argc, char **argv)
{
	AsyncClient client("localhost:50051");

	std::string text;
	while (true)
	{
		std::cin >> text;
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

		client.write_message(text);
		// if (text == "i" || text == "o")
		// 	client.read_message();
	}

	return 0;
}
