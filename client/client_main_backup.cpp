/*

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <services.grpc.pb.h>

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/alarm.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class RouteGuideClient
{
public:
	RouteGuideClient(std::shared_ptr<Channel> channel)
		: stub_(helloworld::Greeter::NewStub(channel))
	{
	}

	void RouteChat()
	{
		class Chatter : public grpc::ClientBidiReactor<helloworld::HelloRequest, helloworld::HelloReply>
		{
		public:
			explicit Chatter(helloworld::Greeter::Stub *stub)
			{
				stub->async()->SayHelloStream(&context_, this);
				// start();
			}
			void start()
			{
				NextWrite();
				StartRead(&response_);
				StartCall();
			}
			void OnWriteDone(bool ok) override 
			{ 
				NextWrite();
			}
			void OnReadDone(bool ok) override
			{
				if (ok)
				{
					std::cout << "Got message " << response_.message() << std::endl;
					StartRead(&response_);
				}
			}
			void OnDone(const Status &s) override
			{
				std::unique_lock<std::mutex> l(mu_);
				status_ = s;
				done_ = true;
				cv_.notify_one();
			}
			Status Await()
			{
				std::unique_lock<std::mutex> l(mu_);
				cv_.wait(l, [this] { return done_; });
				return std::move(status_);
			}

		private:
			void NextWrite()
			{
				if (count_ < 3)
				{
					request_.set_name(std::to_string(count_));
					std::cout << "Sending message " << request_.name() << std::endl;
					StartWrite(&request_);
					count_++;
				}
				else
				{
					StartWritesDone();
				}
			}

			ClientContext context_;
			helloworld::HelloRequest request_;
			helloworld::HelloReply response_;
			int count_ = 0;
			std::mutex mu_;
			std::condition_variable cv_;
			Status status_;
			bool done_ = false;
		};

		Chatter chatter(stub_.get());

		chatter.start();
		Status status = std::move(chatter.Await());

		chatter.start();
		status = std::move(chatter.Await());
		
		if (!status.ok())
		{
			std::cout << "RouteChat rpc failed." << std::endl;
		}
	}

private:
	std::unique_ptr<helloworld::Greeter::Stub> stub_;
};

int main(int argc, char **argv)
{
	RouteGuideClient guide(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
	guide.RouteChat();

	return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <memory>
#include <string>
#include <thread>

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

// NOTE: This is a complex example for an asynchronous, bidirectional streaming
// client. For a simpler example, start with the
// greeter_client/greeter_async_client first.
class AsyncBidiGreeterClient
{
	enum class Type
	{
		READ = 1,
		WRITE = 2,
		CONNECT = 3,
		WRITES_DONE = 4,
		FINISH = 5
	};

public:
	explicit AsyncBidiGreeterClient(std::shared_ptr<Channel> channel)
		: stub_(Greeter::NewStub(channel))
	{
		grpc_thread_.reset(
			new std::thread(std::bind(&AsyncBidiGreeterClient::GrpcThread, this)));
		stream_ = stub_->AsyncSayHelloStream(&context_, &cq_, reinterpret_cast<void *>(Type::CONNECT));
	}

	~AsyncBidiGreeterClient()
	{
		std::cout << "Shutting down client...." << std::endl;
		grpc::Status status;
		cq_.Shutdown();
		grpc_thread_->join();
	}

	// Similar to the async hello example in greeter_async_client but does not
	// wait for the response. Instead queues up a tag in the completion queue
	// that is notified when the server responds back (or when the stream is
	// closed). Returns false when the stream is requested to be closed.
	bool AsyncSayHello(const std::string &user)
	{
		if (user == "quit")
		{
			stream_->WritesDone(reinterpret_cast<void *>(Type::WRITES_DONE));
			return true;
		}

		// Data we are sending to the server.
		request_.set_name(user);

		// This is important: You can have at most one write or at most one read
		// at any given time. The throttling is performed by gRPC completion
		// queue. If you queue more than one write/read, the stream will crash.
		// Because this stream is bidirectional, you *can* have a single read
		// and a single write request queued for the same stream. Writes and reads
		// are independent of each other in terms of ordering/delivery.
		//std::cout << " ** Sending request: " << user << std::endl;
		stream_->Write(request_, reinterpret_cast<void *>(Type::WRITE));
		std::cout << "Client request: " << request_.name() << std::endl;
		return true;
	}

private:
	void AsyncHelloRequestNextMessage()
	{

		// The tag is the link between our thread (main thread) and the completion
		// queue thread. The tag allows the completion queue to fan off
		// notification handlers for the specified read/write requests as they
		// are being processed by gRPC.
		if(request_.name() == "i" ||  request_.name() == "o")
		{
			stream_->Read(&response_, reinterpret_cast<void *>(Type::READ));
			std::cout << "Server response :" << response_.message() << std::endl;
		}
	}

	// Runs a gRPC completion-queue processing thread. Checks for 'Next' tag
	// and processes them until there are no more (or when the completion queue
	// is shutdown).
	void GrpcThread()
	{
		while (true)
		{
			void *got_tag;
			bool ok = false;
			// Block until the next result is available in the completion queue "cq".
			// The return value of Next should always be checked. This return value
			// tells us whether there is any kind of event or the cq_ is shutting
			// down.
			if (!cq_.Next(&got_tag, &ok))
			{
				std::cerr << "Client stream closed. Quitting" << std::endl;
				break;
			}

			// It's important to process all tags even if the ok is false. One might
			// want to deallocate memory that has be reinterpret_cast'ed to void*
			// when the tag got initialized. For our example, we cast an int to a
			// void*, so we don't have extra memory management to take care of.
			if (ok)
			{
				//std::cout << std::endl << "**** Processing completion queue tag " << got_tag << std::endl;
				switch (static_cast<Type>(reinterpret_cast<long>(got_tag)))
				{
				case Type::READ:
					break;

				case Type::WRITE:					
					AsyncHelloRequestNextMessage();
					break;

				case Type::CONNECT:
					std::cout << "Client connected" << std::endl;
					break;
					
				case Type::WRITES_DONE:
					std::cout << "writesdone sent,sleeping 5s" << std::endl;
					stream_->Finish(&finish_status_, reinterpret_cast<void *>(Type::FINISH));
					break;

				case Type::FINISH:
					std::cout << "Client finish status:" << finish_status_.error_code() << ", msg:" << finish_status_.error_message() << std::endl;
					//context_.TryCancel();
					cq_.Shutdown();
					break;

				default:
					std::cerr << "Unexpected tag " << got_tag << std::endl;
					assert(false);
				}
			}
		}
	}

	// Context for the client. It could be used to convey extra information to
	// the server and/or tweak certain RPC behaviors.
	ClientContext context_;

	// The producer-consumer queue we use to communicate asynchronously with the
	// gRPC runtime.
	CompletionQueue cq_;

	// Out of the passed in Channel comes the stub, stored here, our view of the
	// server's exposed services.
	std::unique_ptr<Greeter::Stub> stub_;

	// The bidirectional, asynchronous stream for sending/receiving messages.
	std::unique_ptr<ClientAsyncReaderWriter<HelloRequest, HelloReply>> stream_;

	// Allocated protobuf that holds the response. In real clients and servers,
	// the memory management would a bit more complex as the thread that fills
	// in the response should take care of concurrency as well as memory
	// management.
	HelloRequest request_;
	HelloReply response_;

	// Thread that notifies the gRPC completion queue tags.
	std::unique_ptr<std::thread> grpc_thread_;

	// Finish status when the client is done with the stream.
	grpc::Status finish_status_ = grpc::Status::OK;
};

int main(int argc, char **argv)
{
	AsyncBidiGreeterClient greeter(grpc::CreateChannel(
		"localhost:50051", grpc::InsecureChannelCredentials()));

	std::string text;
	while (true)
	{
		//std::cout << "Enter text (type quit to end): ";
		std::cin >> text;

		// Async RPC call that sends a message and awaits a response.
		if (!greeter.AsyncSayHello(text))
		{
			std::cout << "Quitting." << std::endl;
			break;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

using grpc::Channel;
using grpc::ClientAsyncWriter;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

class AsyncBidiGreeterClient
{
	enum class Type
	{
		READ = 1,
		WRITE = 2,
		CONNECT = 3,
		WRITES_DONE = 4,
		FINISH = 5
	};

public:
	explicit AsyncBidiGreeterClient(std::shared_ptr<Channel> channel)
		: stub_(Greeter::NewStub(channel))
	{
		grpc_thread_.reset(new std::thread(std::bind(&AsyncBidiGreeterClient::GrpcThread, this)));
		stream_ = stub_->PrepareAsyncSayHelloStream(&context_, &response_, &cq_);

		stream_->StartCall(reinterpret_cast<void *>(Type::WRITE));
		stream_->Finish(&finish_status_, reinterpret_cast<void *>(Type::FINISH));
	}

	~AsyncBidiGreeterClient()
	{
		std::cout << "Shutting down client...." << std::endl;
		grpc::Status status;
		cq_.Shutdown();
		grpc_thread_->join();
	}

	bool AsyncSayHello(const std::string &user)
	{
		if (user == "quit")
		{
			stream_->WritesDone(reinterpret_cast<void *>(Type::WRITES_DONE));
			return true;
		}

		request_.set_name(user);
		stream_->Write(request_, reinterpret_cast<void *>(Type::WRITE));
		return true;
	}

private:
	void AsyncHelloRequestNextMessage()
	{
		stream_->Write(request_, reinterpret_cast<void *>(Type::WRITE));

		// stream_->Read(&response_, reinterpret_cast<void*>(Type::READ));
	}

	void GrpcThread()
	{
		while (true)
		{
			void *got_tag;
			bool ok = false;
			// Block until the next result is available in the completion queue "cq".
			// The return value of Next should always be checked. This return value
			// tells us whether there is any kind of event or the cq_ is shutting
			// down.
			if (!cq_.Next(&got_tag, &ok))
			{
				std::cerr << "Client stream closed. Quitting" << std::endl;
				break;
			}

			// It's important to process all tags even if the ok is false. One might
			// want to deallocate memory that has be reinterpret_cast'ed to void*
			// when the tag got initialized. For our example, we cast an int to a
			// void*, so we don't have extra memory management to take care of.
			if (ok)
			{
				//std::cout << std::endl << "**** Processing completion queue tag " << got_tag << std::endl;
				switch (static_cast<Type>(reinterpret_cast<long>(got_tag)))
				{
				case Type::READ:
					break;
				case Type::WRITE:
					AsyncHelloRequestNextMessage();
					break;
				case Type::CONNECT:
					break;
				case Type::WRITES_DONE:
					stream_->Finish(&finish_status_, reinterpret_cast<void *>(Type::FINISH));
					break;
				case Type::FINISH:
					//context_.TryCancel();
					cq_.Shutdown();
					break;
				default:
					assert(false);
				}
			}
		}
	}

	ClientContext context_;
	CompletionQueue cq_;
	std::unique_ptr<Greeter::Stub> stub_;
	std::unique_ptr<ClientAsyncWriter<HelloRequest>> stream_;
	HelloRequest request_;
	HelloReply response_;
	std::unique_ptr<std::thread> grpc_thread_;
	grpc::Status finish_status_ = grpc::Status::OK;
};

*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
int main(int argc, char **argv)
{
	AsyncBidiGreeterClient greeter(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

	std::string text;
	for (int i = 0; i < 100; i++)
	{
		// std::cin >> text;
		text = "id: " + std::to_string(i);
		if (!greeter.AsyncSayHello(text))
		{
			std::cout << "Quitting." << std::endl;
			break;
		}
	}

	return 0;
}

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include <services.grpc.pb.h>

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

class GreeterClient
{
public:
	explicit GreeterClient(std::shared_ptr<Channel> channel)
		: stub_(Greeter::NewStub(channel)) {}

	void SayHello(const std::string &user)
	{
		HelloRequest request;
		request.set_name(user);

		AsyncClientCall *call = new AsyncClientCall;
		call->response_reader = stub_->PrepareAsyncSayHello(&call->context, request, &cq_);
		call->response_reader->StartCall();
		call->response_reader->Finish(&call->reply, &call->status, (void *)call);
	}

	void AsyncCompleteRpc()
	{
		void *got_tag;
		bool ok = false;

		while (cq_.Next(&got_tag, &ok))
		{
			AsyncClientCall *call = static_cast<AsyncClientCall *>(got_tag);
			GPR_ASSERT(ok);

			if (call->status.ok())
				std::cout << "Greeter received: " << call->reply.message() << std::endl;
			else
				std::cout << "RPC failed" << std::endl;

			delete call;
		}
	}

private:
	struct AsyncClientCall
	{
		HelloReply reply;
		ClientContext context;
		Status status;
		std::unique_ptr<ClientAsyncResponseReader<HelloReply>> response_reader;
	};

	std::unique_ptr<Greeter::Stub> stub_;
	CompletionQueue cq_;
};

int main(int argc, char **argv)
{
	GreeterClient greeter(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
	std::thread thread_ = std::thread(&GreeterClient::AsyncCompleteRpc, &greeter);

	for (int i = 0; i < 100; i++)
	{
		std::string user("world " + std::to_string(i));
		greeter.SayHello(user); // The actual RPC call!
	}

	std::cout << "Press control-c to quit" << std::endl << std::endl;
	thread_.join(); // blocks forever

	return 0;
}

*/
