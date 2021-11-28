/*
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <services.grpc.pb.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

using grpc::CallbackServerContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::Status;
using std::chrono::system_clock;

class RouteGuideImpl final : public helloworld::Greeter::CallbackService
{
public:
	grpc::ServerBidiReactor<helloworld::HelloRequest, helloworld::HelloReply> *SayHelloStream(CallbackServerContext *context) override
	{
		class Chatter : public grpc::ServerBidiReactor<helloworld::HelloRequest, helloworld::HelloReply>
		{
		public:
			Chatter(std::mutex *mu)
				: mu_(mu)
			{
				StartRead(&request_);
			}
			void OnDone() override
			{
				// Collect the read_starter thread if needed
				if (read_starter_.joinable())
				{
					read_starter_.join();
				}
				delete this;
			}
			void OnReadDone(bool ok) override
			{
				if (ok)
				{
					// We may need to wait an arbitary amount of time on this mutex
					// and we cannot delay the reaction, so start it in a thread
					// Collect the previous read_starter thread if needed
					if (read_starter_.joinable())
					{
						read_starter_.join();
					}
					read_starter_ = std::thread(
						[this]
						{
							mu_->lock();
							requests_iterator_ = requests_.begin();
							NextWrite();
						});
				}
				else
				{
					Finish(Status::OK);
				}
			}
			void OnWriteDone(bool ok) override
			{
				NextWrite();
			}

		private:
			void NextWrite()
			{
				while (requests_iterator_ != requests_.end())
				{
					const auto &n = *requests_iterator_;
					requests_iterator_++;
					if (n.name() == request_.name())
					{				
						helloworld::HelloReply reply;
						reply.set_message("reply_"+n.name());
						StartWrite(&reply);
						return;
					}
				}

				requests_.push_back(request_);
				mu_->unlock();
				StartRead(&request_);
			}

			std::mutex *mu_;
			std::thread read_starter_;
			helloworld::HelloRequest request_;
			std::vector<helloworld::HelloRequest> requests_;
			std::vector<helloworld::HelloRequest>::iterator requests_iterator_;
			helloworld::HelloReply reply_;
		};
		return new Chatter(&mu_);
	}

private:
	std::mutex mu_;
};

void RunServer()
{
	std::string server_address("0.0.0.0:50051");
	RouteGuideImpl service;

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
}

int main(int argc, char **argv)
{
	RunServer();
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

using grpc::Server;
using grpc::ServerAsyncReaderWriter;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

class CallDataBase
{
public:
	CallDataBase(Greeter::AsyncService *service, ServerCompletionQueue *cq) : service_(service), cq_(cq)
	{
	}

	virtual void Proceed(bool ok) = 0;

protected:
	// The means of communication with the gRPC runtime for an asynchronous
	// server.
	Greeter::AsyncService *service_;
	// The producer-consumer queue where for asynchronous server notifications.
	ServerCompletionQueue *cq_;

	// Context for the rpc, allowing to tweak aspects of it such as the use
	// of compression, authentication, as well as to send metadata back to the
	// client.
	ServerContext ctx_;

	// What we get from the client.
	HelloRequest request_;
	// What we send back to the client.
	HelloReply reply_;
};

class CallDataUnary : CallDataBase
{
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	CallDataUnary(Greeter::AsyncService *service, ServerCompletionQueue *cq) : CallDataBase(service, cq), responder_(&ctx_), status_(CREATE)
	{
		// Invoke the serving logic right away.

		// As part of the initial CREATE state, we *request* that the system
		// start processing SayHello requests. In this request, "this" acts are
		// the tag uniquely identifying the request (so that different CallDataUnary
		// instances can serve different requests concurrently), in this case
		// the memory address of this CallDataUnary instance.

		service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_, (void *)this);
		status_ = PROCESS;
	}

	void Proceed(bool ok)
	{

		if (status_ == PROCESS)
		{
			// Spawn a new CallDataUnary instance to serve new clients while we process
			// the one for this CallDataUnary. The instance will deallocate itself as
			// part of its FINISH state.

			new CallDataUnary(service_, cq_);

			// The actual processing.
			std::string prefix("Hello ");
			reply_.set_message(prefix + request_.name());

			// And we are done! Let the gRPC runtime know we've finished, using the
			// memory address of this instance as the uniquely identifying tag for
			// the event.
			status_ = FINISH;
			responder_.Finish(reply_, Status::OK, (void *)this);
		}
		else
		{
			GPR_ASSERT(status_ == FINISH);
			// Once in the FINISH state, deallocate ourselves (CallDataUnary).
			delete this;
		}
	}

private:
	// The means to get back to the client.
	ServerAsyncResponseWriter<HelloReply> responder_;

	// Let's implement a tiny state machine with the following states.
	enum CallStatus
	{
		CREATE,
		PROCESS,
		FINISH
	};
	CallStatus status_; // The current serving state.
};

class CallDataBidi : CallDataBase
{

public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	CallDataBidi(Greeter::AsyncService *service, ServerCompletionQueue *cq) : CallDataBase(service, cq), rw_(&ctx_)
	{
		// Invoke the serving logic right away.

		status_ = BidiStatus::CONNECT;

		ctx_.AsyncNotifyWhenDone((void *)this);
		service_->RequestSayHelloStream(&ctx_, &rw_, cq_, cq_, (void *)this);
	}

	void Proceed(bool ok)
	{

		std::unique_lock<std::mutex> _wlock(this->m_mutex);

		switch (status_)
		{
		case BidiStatus::READ:

			//Meaning client said it wants to end the stream either by a 'writedone' or 'finish' call.
			if (!ok)
			{
				std::cout << "thread:" << std::this_thread::get_id() << " tag:" << this << " CQ returned false." << std::endl;
				Status _st(StatusCode::OUT_OF_RANGE, "test error msg");
				rw_.Finish(_st, (void *)this);
				status_ = BidiStatus::DONE;
				std::cout << "thread:" << std::this_thread::get_id() << " tag:" << this << " after call Finish(), cancelled:" << this->ctx_.IsCancelled() << std::endl;
				break;
			}

			std::cout << "thread:" << std::this_thread::get_id() << " tag:" << this << " Read a new message: " << request_.name() << std::endl;

			if(request_.name() == "i")
			{
				reply_.set_message("reply - i");
				rw_.Write(reply_, (void *)this);
				status_ = BidiStatus::WRITE;
				break;
			}

			if(request_.name() == "o")
			{
				reply_.set_message("reply - o");
				rw_.Write(reply_, (void *)this);
				status_ = BidiStatus::WRITE;
				break;
			}

			rw_.Read(&request_, (void *)this);
			status_ = BidiStatus::READ;

			// reply_.set_message("reply" + request_.name());
			// rw_.Write(reply_, (void *)this);
			//status_ = BidiStatus::WRITE;

			break;

		case BidiStatus::WRITE:
			std::cout << "thread:" << std::this_thread::get_id() << " tag:" << this << " Written a message:" << reply_.message() << std::endl;
			rw_.Read(&request_, (void *)this);
			status_ = BidiStatus::READ;
			break;

		case BidiStatus::CONNECT:
			std::cout << "thread:" << std::this_thread::get_id() << " tag:" << this << " connected:" << std::endl;
			new CallDataBidi(service_, cq_);
			rw_.Read(&request_, (void *)this);
			status_ = BidiStatus::READ;
			break;

		case BidiStatus::DONE:
			std::cout << "thread:" << std::this_thread::get_id() << " tag:" << this << " Server done, cancelled:" << this->ctx_.IsCancelled() << std::endl;
			status_ = BidiStatus::FINISH;
			break;

		case BidiStatus::FINISH:
			std::cout << "thread:" << std::this_thread::get_id() << "tag:" << this << " Server finish, cancelled:" << this->ctx_.IsCancelled() << std::endl;
			_wlock.unlock();
			delete this;
			break;

		default:
			std::cerr << "Unexpected tag " << int(status_) << std::endl;
			assert(false);
		}
	}

private:
	// The means to get back to the client.
	ServerAsyncReaderWriter<HelloReply, HelloRequest> rw_;

	// Let's implement a tiny state machine with the following states.
	enum class BidiStatus
	{
		READ = 1,
		WRITE = 2,
		CONNECT = 3,
		DONE = 4,
		FINISH = 5
	};
	BidiStatus status_;

	std::mutex m_mutex;
};

class ServerImpl final
{
public:
	~ServerImpl()
	{
		server_->Shutdown();
		// Always shutdown the completion queue after the server.
		for (const auto &_cq : m_cq)
			_cq->Shutdown();
	}

	// There is no shutdown handling in this code.
	void Run()
	{
		std::string server_address("0.0.0.0:" + std::to_string(50051));

		ServerBuilder builder;
		// Listen on the given address without any authentication mechanism.
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		// Register "service_" as the instance through which we'll communicate with
		// clients. In this case it corresponds to an *asynchronous* service.
		builder.RegisterService(&service_);
		// Get hold of the completion queue used for the asynchronous communication
		// with the gRPC runtime.

		for (int i = 0; i < 1; ++i)
		{
			//cq_ = builder.AddCompletionQueue();
			m_cq.emplace_back(builder.AddCompletionQueue());
		}

		// Finally assemble the server.
		server_ = builder.BuildAndStart();
		std::cout << "Server listening on " << server_address << std::endl;

		// Proceed to the server's main loop.
		std::vector<std::thread *> _vec_threads;

		for (int i = 0; i < 1; ++i)
		{
			int _cq_idx = i % 1;
			for (int j = 0; j < 1; ++j)
			{
				new CallDataUnary(&service_, m_cq[_cq_idx].get());
				new CallDataBidi(&service_, m_cq[_cq_idx].get());
			}

			_vec_threads.emplace_back(new std::thread(&ServerImpl::HandleRpcs, this, _cq_idx));
		}

		std::cout << 1 << " working aysnc threads spawned" << std::endl;

		for (const auto &_t : _vec_threads)
			_t->join();
	}

private:
	// Class encompasing the state and logic needed to serve a request.
	// This can be run in multiple threads if needed.
	void HandleRpcs(int cq_idx)
	{
		// Spawn a new CallDataUnary instance to serve new clients.
		void *tag; // uniquely identifies a request.
		bool ok;
		while (true)
		{
			// Block waiting to read the next event from the completion queue. The
			// event is uniquely identified by its tag, which in this case is the
			// memory address of a CallDataUnary instance.
			// The return value of Next should always be checked. This return value
			// tells us whether there is any kind of event or cq_ is shutting down.
			//GPR_ASSERT(cq_->Next(&tag, &ok));
			GPR_ASSERT(m_cq[cq_idx]->Next(&tag, &ok));

			CallDataBase *_p_ins = (CallDataBase *)tag;
			_p_ins->Proceed(ok);
		}
	}

	std::vector<std::unique_ptr<ServerCompletionQueue>> m_cq;

	Greeter::AsyncService service_;
	std::unique_ptr<Server> server_;
};

int main(int argc, char **argv)
{
	ServerImpl server;
	server.Run();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <services.grpc.pb.h>

using grpc::Server;
using grpc::ServerAsyncReader;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

enum class Type
{
	READ = 1,
	WRITE = 2,
	CONNECT = 3,
	DONE = 4,
	FINISH = 5
};

// NOTE: This is a complex example for an asynchronous, bidirectional streaming
// server. For a simpler example, start with the
// greeter_server/greeter_async_server first.

// Most of the logic is similar to AsyncBidiGreeterClient, so follow that class
// for detailed comments. Two main differences between the server and the client
// are: (a) Server cannot initiate a connection, so it first waits for a
// 'connection'. (b) Server can handle multiple streams at the same time, so
// the completion queue/server have a longer lifetime than the client(s).
class AsyncBidiGreeterServer
{
public:
	AsyncBidiGreeterServer()
	{
		// In general avoid setting up the server in the main thread (specifically,
		// in a constructor-like function such as this). We ignore this in the
		// context of an example.
		std::string server_address("0.0.0.0:50051");

		ServerBuilder builder;
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		builder.RegisterService(&service_);
		cq_ = builder.AddCompletionQueue();
		server_ = builder.BuildAndStart();

		// This initiates a single stream for a single client. To allow multiple
		// clients in different threads to connect, simply 'request' from the
		// different threads. Each stream is independent but can use the same
		// completion queue/context objects.
		stream_.reset(new ServerAsyncReader<HelloReply, HelloRequest>(&context_));
		service_.RequestSayHelloStream(&context_, stream_.get(), cq_.get(), cq_.get(), reinterpret_cast<void *>(Type::CONNECT));

		// This is important as the server should know when the client is done.
		context_.AsyncNotifyWhenDone(reinterpret_cast<void *>(Type::DONE));

		grpc_thread_.reset(new std::thread( (std::bind(&AsyncBidiGreeterServer::GrpcThread, this))));
		std::cout << "Server listening on " << server_address << std::endl;
	}

	void SetResponse(const std::string &response)
	{
		if (response == "quit" && IsRunning())
		{
			HelloReply reply;
			reply.set_message(response);
			stream_->Finish(reply, grpc::Status::OK, reinterpret_cast<void *>(Type::FINISH));
		}
		response_str_ = response;
	}

	~AsyncBidiGreeterServer()
	{
		std::cout << "Shutting down server...." << std::endl;
		server_->Shutdown();
		// Always shutdown the completion queue after the server.
		cq_->Shutdown();
		grpc_thread_->join();
	}

	bool IsRunning() const { return is_running_; }

private:
	void AsyncWaitForHelloRequest()
	{
		if (IsRunning())
		{
			// In the case of the server, we wait for a READ first and then write a
			// response. A server cannot initiate a connection so the server has to
			// wait for the client to send a message in order for it to respond back.
			stream_->Read(&request_, reinterpret_cast<void *>(Type::READ));
		}
	}

	void AsyncHelloSendResponse()
	{
		std::cout << " ** Handling request: " << request_.name() << std::endl;
		HelloReply response;
		std::cout << " ** Sending response: " << response_str_ << std::endl;
		response.set_message(response_str_);
	}

	void GrpcThread()
	{
		while (true)
		{
			void *got_tag = nullptr;
			bool ok = false;
			if (!cq_->Next(&got_tag, &ok))
			{
				std::cerr << "Server stream closed. Quitting" << std::endl;
				break;
			}

			//assert(ok);

			if (ok)
			{
				std::cout << std::endl
						  << "**** Processing completion queue tag " << got_tag
						  << std::endl;
				switch (static_cast<Type>(reinterpret_cast<size_t>(got_tag)))
				{
				case Type::READ:
					std::cout << "Read a new message." << std::endl;
					AsyncHelloSendResponse();
					break;
				case Type::WRITE:
					std::cout << "Sending message (async)." << std::endl;
					AsyncWaitForHelloRequest();
					break;
				case Type::CONNECT:
					std::cout << "Client connected." << std::endl;
					AsyncWaitForHelloRequest();
					break;
				case Type::DONE:
					std::cout << "Server disconnecting." << std::endl;
					is_running_ = false;
					break;
				case Type::FINISH:
					std::cout << "Server quitting." << std::endl;
					break;
				default:
					std::cerr << "Unexpected tag " << got_tag << std::endl;
					assert(false);
				}
			}
		}
	}

private:
	HelloRequest request_;
	std::string response_str_ = "Default server response";
	ServerContext context_;
	Greeter::AsyncService service_;
	std::unique_ptr<ServerCompletionQueue> cq_;
	std::unique_ptr<Server> server_;
	std::unique_ptr<ServerAsyncReader<HelloReply, HelloRequest>> stream_;
	std::unique_ptr<std::thread> grpc_thread_;
	bool is_running_ = true;
};

int main(int argc, char **argv)
{
	AsyncBidiGreeterServer server;

	std::string response;
	while (server.IsRunning())
	{
		std::cout << "Enter next set of responses (type quit to end): ";
		std::cin >> response;
		server.SetResponse(response);
	}

	return 0;
}

*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include <services.grpc.pb.h>

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

class ServerImpl final
{
public:
  ~ServerImpl()
  {
    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    cq_->Shutdown();
  }

  // There is no shutdown handling in this code.
  void Run()
  {
    std::string server_address("0.0.0.0:50051");

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service_" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *asynchronous* service.
    builder.RegisterService(&service_);
    // Get hold of the completion queue used for the asynchronous communication
    // with the gRPC runtime.
    cq_ = builder.AddCompletionQueue();
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    // Proceed to the server's main loop.
    HandleRpcs();
  }

private:
  // Class encompasing the state and logic needed to serve a request.
  class CallData
  {
  public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    CallData(Greeter::AsyncService *service, ServerCompletionQueue *cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE)
    {
      // Invoke the serving logic right away.
      Proceed();
    }

    void Proceed()
    {
      if (status_ == CREATE)
      {
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;

        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_, this);
      }
      else if (status_ == PROCESS)
      {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(service_, cq_);

        // The actual processing.
        std::string prefix("Hello ");
        reply_.set_message(prefix + request_.name());

        // And we are done! Let the gRPC runtime know we've finished, using the
        // memory address of this instance as the uniquely identifying tag for
        // the event.
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
      }
      else
      {
        GPR_ASSERT(status_ == FINISH);
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this;
      }
    }

  private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    Greeter::AsyncService *service_;
    // The producer-consumer queue where for asynchronous server notifications.
    ServerCompletionQueue *cq_;
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    ServerContext ctx_;

    // What we get from the client.
    HelloRequest request_;
    // What we send back to the client.
    HelloReply reply_;

    // The means to get back to the client.
    ServerAsyncResponseWriter<HelloReply> responder_;

    // Let's implement a tiny state machine with the following states.
    enum CallStatus
    {
      CREATE,
      PROCESS,
      FINISH
    };
    CallStatus status_; // The current serving state.
  };

  // This can be run in multiple threads if needed.
  void HandleRpcs()
  {
    // Spawn a new CallData instance to serve new clients.
    new CallData(&service_, cq_.get());
    void *tag; // uniquely identifies a request.
    bool ok;
    while (true)
    {
      // Block waiting to read the next event from the completion queue. The
      // event is uniquely identified by its tag, which in this case is the
      // memory address of a CallData instance.
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or cq_ is shutting down.
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      static_cast<CallData *>(tag)->Proceed();
    }
  }

  // grpc::ServerAsyncReader<HelloRequest, HelloReply> r;

  std::unique_ptr<ServerCompletionQueue> cq_;
  Greeter::AsyncService service_;
  std::unique_ptr<Server> server_;
};

int main(int argc, char **argv)
{
  std::cout << grpc::Version() << std::endl;
  ServerImpl server;
  server.Run();
  return 0;
}

*/