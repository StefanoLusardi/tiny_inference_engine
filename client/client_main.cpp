#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <condition_variable>

#include <grpcpp/grpcpp.h>
#include <services.grpc.pb.h>

using grpc::Channel;
using grpc::ClientAsyncReaderWriter;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

#define TAG(value) reinterpret_cast<void *>(value)

enum class Type : unsigned long
{
	READ = 1,
	WRITE = 2,
	CONNECT = 3,
	DONE = 4,
	FINISH = 5
};

struct AsyncClientCall
{
	HelloReply reply;
	ClientContext context;
	Status status;
	std::unique_ptr<grpc::ClientAsyncResponseReader<HelloReply>> response_reader;
};

class AsyncClient final
{
public:
	explicit AsyncClient(const std::string &channel_address)
		: stub_{Greeter::NewStub(grpc::CreateChannel(channel_address, grpc::InsecureChannelCredentials()))},
		  grpc_thread_bidi_{std::make_unique<std::thread>([this]
														  { grpc_thread_bidi(); })},
		  grpc_thread_unary_{std::make_unique<std::thread>([this]
														   { grpc_thread_unary(); })},
		  stream_{stub_->AsyncSayHelloStream(&context_, &cq_, TAG(Type::CONNECT))},
		  finish_status_{grpc::Status::OK},
		  is_finished{false}
	{
	}

	~AsyncClient()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_cv.wait(lock, [this]
				 { return is_finished; });

		std::cout << "Shutting down client" << std::endl;
		cq_.Shutdown();
		unary_cq_.Shutdown();

		if (grpc_thread_unary_->joinable())
			grpc_thread_unary_->join();

		if (grpc_thread_bidi_->joinable())
			grpc_thread_bidi_->join();
	}

	void quit()
	{
		stream_->WritesDone(TAG(Type::DONE));
	}

	void write_message(const std::string &msg)
	{
		request_.set_name(msg);
		stream_->Write(request_, TAG(Type::WRITE));
	}

	void read_message()
	{
		stream_->Read(&response_, TAG(Type::READ));
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

		AsyncClientCall *call = new AsyncClientCall;
		call->response_reader = stub_->PrepareAsyncSayHello(&call->context, request, &unary_cq_);
		call->response_reader->StartCall();
		call->response_reader->Finish(&call->reply, &call->status, (void *)call);
	}

private:
	void grpc_thread_bidi()
	{
		while (true)
		{
			void *status_tag;
			bool ok = false;

			if (!cq_.Next(&status_tag, &ok))
			{
				std::cerr << "Client stream closed" << std::endl;
				break;
			}

			if (ok)
			{
				Type status = static_cast<Type>(reinterpret_cast<unsigned long>(status_tag));
				proceed(status);
			}
		}
	}

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

			if (ok)
			{

			if (call->status.ok())
				std::cout << call->reply.message() << std::endl;
			else
				std::cout << "RPC failed: (" << call->status.error_code() << ") " << call->status.error_message() << std::endl;
			}

			delete call;
		}
	}

	void proceed(Type status)
	{
		switch (status)
		{
		case Type::READ:
			std::cout << "Server response :" << response_.message() << std::endl;
			break;

		case Type::WRITE:
			std::cout << "Client request: " << request_.name() << std::endl;
			break;

		case Type::CONNECT:
			std::cout << "Client connected" << std::endl;
			break;

		case Type::DONE:
			std::cout << "Client stream finished" << std::endl;
			stream_->Finish(&finish_status_, TAG(Type::FINISH));
			break;

		case Type::FINISH:
		{
			std::cout << "Client disconnectd" << std::endl;
			std::cout << "RPC status: (" << finish_status_.error_code() << ") " << finish_status_.error_message() << std::endl;

			std::lock_guard<std::mutex> lock(_mutex);
			is_finished = true;
			_cv.notify_one();

			break;
		}

		default:
			std::cerr << "Unexpected tag " << (int)status << std::endl;
			assert(false);
		}
	}

	std::mutex _mutex;
	std::condition_variable _cv;
	bool is_finished;

	ClientContext context_;
	CompletionQueue cq_;
	CompletionQueue unary_cq_;
	std::unique_ptr<Greeter::Stub> stub_;
	std::unique_ptr<ClientAsyncReaderWriter<HelloRequest, HelloReply>> stream_;
	HelloRequest request_;
	HelloReply response_;
	std::unique_ptr<std::thread> grpc_thread_bidi_;
	std::unique_ptr<std::thread> grpc_thread_unary_;
	grpc::Status finish_status_;
};

int main(int argc, char **argv)
{
	AsyncClient client("192.168.1.59:50051");
	// AsyncClient client("localhost:50051");

	std::string text;
	while (true)
	{
		std::cin >> text;
		if (text == "quit")
		{
			client.quit();
			return 0;
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

		client.write_message(text);
		if (text == "i" || text == "o")
			client.read_message();
	}

	return 0;
}