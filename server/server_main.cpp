#include <cstddef>
#include <functional>
#include <grpcpp/impl/codegen/status.h>
#include <ios>
#include <mutex>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <condition_variable>

// TODO: check includes
#include <grpcpp/grpcpp.h>
#include <services.grpc.pb.h>

using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

using namespace std::chrono_literals;

class AsyncServerCall
{
public:
	AsyncServerCall(const std::shared_ptr<Greeter::AsyncService>& service, const std::shared_ptr<grpc::ServerCompletionQueue>& cq)
		: _service{service}, _cq{cq}
	{
	}

	virtual bool process(bool ok) = 0;

protected:
	std::shared_ptr<Greeter::AsyncService> _service;
	std::shared_ptr<grpc::ServerCompletionQueue> _cq;
	grpc::ServerContext _context;
};

template<class RequestT, class ResponseT>
class AsyncServerCallback : public AsyncServerCall
{
public:
	AsyncServerCallback(const std::shared_ptr<Greeter::AsyncService>& service, const std::shared_ptr<grpc::ServerCompletionQueue>& cq)
		: AsyncServerCall(service, cq) {}

protected:
	RequestT _request;
	ResponseT _reply;
};

template<class RequestT, class ResponseT>
class AsyncServerUnaryCall : public AsyncServerCallback<RequestT, ResponseT>
{
public:
	AsyncServerUnaryCall(const std::shared_ptr<Greeter::AsyncService>& service, const std::shared_ptr<grpc::ServerCompletionQueue>& cq)
		: AsyncServerCallback<RequestT, ResponseT>(service, cq)
		, _rpc{&this->_context}
		, _call_state{CREATE}
	{
		this->_service->RequestSayHello(&this->_context, &this->_request, &_rpc, this->_cq.get(), this->_cq.get(), (void *)this);
		_call_state = PROCESS;
	}

	bool process(bool ok) override
	{
		if(!ok)
		{
			// _call_state = CallState::FINISH;
			// _rpc.Finish(this->_reply, grpc::Status::CANCELLED, (void *)this);

			delete this;
			return false;
		}

		std::unique_lock<std::mutex> lock(_mutex);

		switch (_call_state)
		{
			case PROCESS:
			{
				std::cout << "thread: " << std::this_thread::get_id() << " PROCESS" << std::endl;
				new AsyncServerUnaryCall(this->_service, this->_cq);
				this->_reply.set_message("Hello " + this->_request.name());

				std::this_thread::sleep_for(3s);

				_call_state = FINISH;
				_rpc.Finish(this->_reply, grpc::Status::OK, (void *)this);
				break;
			}

			case FINISH:
				std::cout << "thread: " << std::this_thread::get_id() << " FINISH" << std::endl;
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
	enum CallState
	{
		CREATE,
		PROCESS,
		FINISH
	};

	grpc::ServerAsyncResponseWriter<HelloReply> _rpc;
	CallState _call_state;
	std::mutex _mutex;
};

template<class RequestT, class ResponseT>
class AsyncServerBidiCall : AsyncServerCallback<RequestT, ResponseT>
{
public:
	AsyncServerBidiCall(const std::shared_ptr<Greeter::AsyncService>& service, const std::shared_ptr<grpc::ServerCompletionQueue>& cq)
		: AsyncServerCallback<RequestT, ResponseT>(service, cq)
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
				_rpc.Finish(grpc::Status::OK, (void *)this);
				_call_state = CallState::DONE;
				break;
			}

			if (this->_request.name() == "last")
			{
				this->_reply.set_message("Server Reply");
				std::cout << "thread: " << std::this_thread::get_id() << " tag: " << this << " - WRITE" << " - Reply: " << this->_reply.message() << std::endl;
				
				std::this_thread::sleep_for(3s);
				_rpc.Write(this->_reply, (void *)this);
				_call_state = CallState::WRITE;
				break;
			}

			std::cout << "thread: " << std::this_thread::get_id() << " tag: " << this << " - READ" << " - Request: " << this->_request.name() << std::endl;
			_rpc.Read(&this->_request, (void *)this);
			_call_state = CallState::READ;
			break;
		}

		case CallState::WRITE:
			std::cout << "thread: " << std::this_thread::get_id() << " tag: " << this << " - WRITE" << " - Reply: " << this->_reply.message() << std::endl;
			_rpc.Read(&this->_request, (void *)this);
			_call_state = CallState::READ;
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
			new AsyncServerBidiCall<RequestT, ResponseT>(this->_service, this->_cq);
			_rpc.Read(&this->_request, (void *)this);
			_call_state = CallState::READ;
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
	enum class CallState
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

class AsyncServer final
{
public:
	explicit AsyncServer()
	{
		const std::string address = get_environment_variable("SERVER_ADDRESS", "0.0.0.0");
		const std::string port = get_environment_variable("SERVER_PORT", "50051");
		_server_uri = address + ":" + port;		
		_num_threads_bidi_call = std::stoi(get_environment_variable("NUM_THREADS_BIDI", "1"));
		_num_threads_unary_call = std::stoi(get_environment_variable("NUM_THREADS_UNARY", "4"));
	}

	~AsyncServer()
	{
		if(_is_running)
			stop();
	}

	const char* get_environment_variable(const char* key, const char* default_value = nullptr) noexcept
	{
		const char* var = std::getenv(key);
		return var ? var : default_value;
	}

	void start()
	{
		_service = std::make_shared<Greeter::AsyncService>();

		grpc::ServerBuilder builder;
		builder.AddListeningPort(_server_uri, grpc::InsecureServerCredentials());
		builder.RegisterService(_service.get());

		// builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 1000);
		// builder.SetDefaultCompressionLevel(GRPC_COMPRESS_LEVEL_HIGH);
		// builder.SetDefaultCompressionAlgorithm(GRPC_COMPRESS_GZIP);

		// const auto num_cq_bidi_call = _num_threads_bidi_call;
		// for (auto i = 0; i < num_cq_bidi_call; ++i)
		// 	_completion_queues.emplace_back(builder.AddCompletionQueue());

		// const auto num_cq_unary_call = _num_threads_unary_call;
		// for (auto i = 0; i < num_cq_unary_call; ++i)
		// 	_completion_queues.emplace_back(builder.AddCompletionQueue());

		for (auto i = 0; i < 2; ++i)
			_completion_queues.emplace_back(builder.AddCompletionQueue());

		_server = builder.BuildAndStart();

		for (auto thread_idx = 0; thread_idx < _num_threads_bidi_call; ++thread_idx)
		{
			const auto cq = _completion_queues[0];
			new AsyncServerBidiCall<HelloRequest, HelloReply>(_service, cq);
			_server_threads.emplace_back(std::thread([this](const std::shared_ptr<grpc::ServerCompletionQueue>& cq){ grpc_thread_worker(cq); }, cq));
		}

		for (auto thread_idx = 0; thread_idx < _num_threads_unary_call; ++thread_idx)
		{
			const auto cq = _completion_queues[1];
			new AsyncServerUnaryCall<HelloRequest, HelloReply>(_service, cq);
			_server_threads.emplace_back(std::thread([this](const std::shared_ptr<grpc::ServerCompletionQueue>& cq){ grpc_thread_worker(cq); }, cq));
		}

		std::cout << "Server running @ " << _server_uri << std::endl;
		std::cout << "num_threads_unary_call: " << _num_threads_unary_call << std::endl;
		std::cout << "num_threads_bidi_call: " << _num_threads_bidi_call << std::endl;

		_is_running = true;
	}

	void run()
	{
		_server->Wait();		
		std::unique_lock<std::mutex> lock(_shutdown_mutex);
		_shutdown_cv.wait(lock, [this]{return !_is_running.load();});
	}

	void stop()
	{
		_server->Shutdown();

		for (auto& t : _server_threads)
			if(t.joinable())
				t.join();

		for (auto&& cq : _completion_queues)
			cq->Shutdown();

		// Drain Completion Queues
		void* drain_tag = nullptr;
    	bool ok = false;
		for (auto&& cq : _completion_queues)
    		while (cq->Next(&drain_tag, &ok));

		_is_running = false;
		_shutdown_cv.notify_one();
	}

private:
	void grpc_thread_worker(const std::shared_ptr<grpc::ServerCompletionQueue>& cq)
	{
		while (_is_running)
		{
			void *call_tag = nullptr;
			bool ok = false;
			if (!cq->Next(&call_tag, &ok))
			{
				std::cerr << "Server stream closed. Quitting" << std::endl;
				break;
			}

			auto call = static_cast<AsyncServerCall*>(call_tag);
			if (!call->process(ok))
				break;
		}
	}

	std::vector<std::thread> _server_threads;
	std::vector<std::shared_ptr<grpc::ServerCompletionQueue>> _completion_queues;
	std::shared_ptr<Greeter::AsyncService> _service;
	std::unique_ptr<grpc::Server> _server;

	std::atomic_bool _is_running;
	std::mutex _shutdown_mutex;
	std::condition_variable _shutdown_cv;

	std::string _server_uri;
	unsigned int _num_threads_unary_call;
	unsigned int _num_threads_bidi_call;
};

int main(int argc, char **argv)
{
	AsyncServer server;
	server.start();

	// std::thread stop_thread([&server]
	// {
	// 	std::this_thread::sleep_for(1s);
	// 	server.stop();
	// });

	server.run();

	// stop_thread.join();

	return 0;
}
