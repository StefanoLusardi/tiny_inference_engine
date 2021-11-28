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

using namespace std::chrono_literals;

class CallDataBase
{
public:
	CallDataBase(Greeter::AsyncService *service, ServerCompletionQueue *cq)
		: service_{service}, cq_{cq}
	{
	}

	virtual void proceed(bool ok) = 0;

protected:
	Greeter::AsyncService *service_;
	ServerCompletionQueue *cq_;
	ServerContext ctx_;
	HelloRequest request_;
	HelloReply reply_;
};

class CallDataUnary : CallDataBase
{
public:
	CallDataUnary(Greeter::AsyncService *service, ServerCompletionQueue *cq)
		: CallDataBase(service, cq), responder_{&ctx_}, status_{CREATE}
	{
		service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_, (void *)this);
		status_ = PROCESS;
	}

	void proceed(bool ok)
	{
		switch (status_)
		{
		case PROCESS:
		{
			new CallDataUnary(service_, cq_);
			reply_.set_message("Hello " + request_.name());

			std::this_thread::sleep_for(3s);

			status_ = FINISH;
			responder_.Finish(reply_, Status::OK, (void *)this);
			break;
		}

		case FINISH:
			delete this;
			break;

		default:
			std::cerr << "Unexpected tag " << int(status_) << std::endl;
			assert(false);
		}
	}

private:
	enum CallStatus
	{
		CREATE,
		PROCESS,
		FINISH
	};

	CallStatus status_;
	ServerAsyncResponseWriter<HelloReply> responder_;
};

class CallDataBidi : CallDataBase
{
public:
	CallDataBidi(Greeter::AsyncService *service, ServerCompletionQueue *cq)
		: CallDataBase(service, cq), rw_{&ctx_}, status_{BidiStatus::CONNECT}
	{
		ctx_.AsyncNotifyWhenDone((void *)this);
		service_->RequestSayHelloStream(&ctx_, &rw_, cq_, cq_, (void *)this);
	}

	void proceed(bool ok)
	{
		std::unique_lock<std::mutex> lock(_mutex);

		switch (status_)
		{
		case BidiStatus::READ:

			if (!ok)
			{
				//Meaning client said it wants to end the stream either by a 'writedone' or 'finish' call.
				std::cout << "thread:" << std::this_thread::get_id() << " tag:" << this << "Close stream" << std::endl;
				Status _st = Status::OK;
				rw_.Finish(_st, (void *)this);

				status_ = BidiStatus::DONE;
				break;
			}

			std::cout << "thread:" << std::this_thread::get_id() << " tag:" << this << " Read: " << request_.name() << std::endl;

			if (request_.name() == "i")
			{
				std::this_thread::sleep_for(3s);
				reply_.set_message("reply - i");
				rw_.Write(reply_, (void *)this);
				status_ = BidiStatus::WRITE;
				break;
			}

			if (request_.name() == "o")
			{
				std::this_thread::sleep_for(3s);
				reply_.set_message("reply - o");
				rw_.Write(reply_, (void *)this);
				status_ = BidiStatus::WRITE;
				break;
			}

			rw_.Read(&request_, (void *)this);
			status_ = BidiStatus::READ;

			break;

		case BidiStatus::WRITE:
			std::cout << "thread:" << std::this_thread::get_id() << " tag:" << this << " Write: " << reply_.message() << std::endl;
			rw_.Read(&request_, (void *)this);
			status_ = BidiStatus::READ;
			break;

		case BidiStatus::CONNECT:
			std::cout << "thread:" << std::this_thread::get_id() << " tag:" << this << " New client connected:" << std::endl;
			new CallDataBidi(service_, cq_);
			rw_.Read(&request_, (void *)this);
			status_ = BidiStatus::READ;
			break;

		case BidiStatus::DONE:
			std::cout << "thread:" << std::this_thread::get_id() << " tag:" << this << " Client stream finished , cancelled:" << this->ctx_.IsCancelled() << std::endl;
			status_ = BidiStatus::FINISH;
			break;

		case BidiStatus::FINISH:
			std::cout << "thread:" << std::this_thread::get_id() << "tag:" << this << " Client disconnected, cancelled:" << this->ctx_.IsCancelled() << std::endl;
			lock.unlock();
			delete this;
			break;

		default:
			std::cerr << "Unexpected tag " << int(status_) << std::endl;
			assert(false);
		}
	}

private:
	enum class BidiStatus
	{
		READ = 1,
		WRITE = 2,
		CONNECT = 3,
		DONE = 4,
		FINISH = 5
	};

	ServerAsyncReaderWriter<HelloReply, HelloRequest> rw_;
	BidiStatus status_;
	std::mutex _mutex;
};

class AsyncServer final
{
public:
	~AsyncServer()
	{
		server_->Shutdown();
		for (auto &&cq : _completion_queues)
			cq->Shutdown();
	}

	void run()
	{
		// There is no shutdown handling in this code.
		std::string server_address("0.0.0.0:" + std::to_string(50051));

		ServerBuilder builder;
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		builder.RegisterService(&service_);

		// DEBUG
		const auto num_threads = 1;
		const auto num_cq = 1;
		const auto num_rpcs = 1;

		// const auto num_threads = std::thread::hardware_concurrency();
		// const auto num_cq = num_threads / 2;
		// const auto num_rpcs = 2;

		for (auto i = 0; i < num_cq; ++i)
		{
			_completion_queues.emplace_back(builder.AddCompletionQueue());
		}

		server_ = builder.BuildAndStart();

		std::vector<std::thread> _server_threads;
		for (auto thread_idx = 0; thread_idx < num_threads; ++thread_idx)
		{
			int cq_idx = thread_idx % num_cq;
			for (auto rpcs_idx = 0; rpcs_idx < num_rpcs; ++rpcs_idx)
			{
				new CallDataUnary(&service_, _completion_queues[cq_idx].get());
				new CallDataBidi(&service_, _completion_queues[cq_idx].get());
			}

			_server_threads.emplace_back(std::thread([this](int cq_idx)
													 { grpc_thread(cq_idx); },
													 cq_idx));
		}

		std::cout << "Server address: " << server_address << std::endl;
		std::cout << "Worker threads: " << num_threads << std::endl;

		for (auto &&t : _server_threads)
			t.join();
	}

private:
	void grpc_thread(int cq_idx)
	{
		while (true)
		{
			void *call_tag;
			bool ok = false;
			if (!_completion_queues[cq_idx]->Next(&call_tag, &ok))
			{
				std::cerr << "Server stream closed. Quitting" << std::endl;
				break;
			}

			auto call = static_cast<CallDataBase *>(call_tag);
			call->proceed(ok);
		}
	}

	std::vector<std::unique_ptr<ServerCompletionQueue>> _completion_queues;
	Greeter::AsyncService service_;
	std::unique_ptr<Server> server_;
};

int main(int argc, char **argv)
{
	AsyncServer server;
	server.run();
	return 0;
}
