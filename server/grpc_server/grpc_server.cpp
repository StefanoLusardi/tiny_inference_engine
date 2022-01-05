#include <cstddef>
#include <functional>
#include <ios>
#include <mutex>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <condition_variable>
// TODO: check includes

#include "grpc_server.hpp"
#include <spdlog/spdlog.h>

using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

using namespace std::chrono_literals;

namespace xyz
{
class AsyncServerCall
{
public:
	AsyncServerCall(const std::shared_ptr<Greeter::AsyncService>& service, const std::shared_ptr<grpc::ServerCompletionQueue>& cq, const std::shared_ptr<engine::engine_interface>& engine_ptr)
		: _service{service}, _cq{cq}, _engine_ptr{engine_ptr} {}

	virtual bool process(bool ok) = 0;

protected:
	std::shared_ptr<Greeter::AsyncService> _service;
	std::shared_ptr<grpc::ServerCompletionQueue> _cq;
	std::shared_ptr<engine::engine_interface> _engine_ptr;
	grpc::ServerContext _context;
};

template<class RequestT, class ResponseT>
class AsyncServerCallback : public AsyncServerCall
{
public:
	AsyncServerCallback(const std::shared_ptr<Greeter::AsyncService>& service, const std::shared_ptr<grpc::ServerCompletionQueue>& cq, const std::shared_ptr<engine::engine_interface>& engine_ptr)
		: AsyncServerCall(service, cq, engine_ptr) {}

protected:
	RequestT _request;
	ResponseT _response;
};

template<class RequestT, class ResponseT>
class AsyncServerUnaryCall : public AsyncServerCallback<RequestT, ResponseT>
{
public:
	AsyncServerUnaryCall(const std::shared_ptr<Greeter::AsyncService>& service, const std::shared_ptr<grpc::ServerCompletionQueue>& cq, const std::shared_ptr<engine::engine_interface>& engine_ptr)
		: AsyncServerCallback<RequestT, ResponseT>(service, cq, engine_ptr)
		, _rpc{&this->_context}
		, _call_state{CallState::CREATE}
	{
		this->_service->RequestSayHello(&this->_context, &this->_request, &_rpc, this->_cq.get(), this->_cq.get(), (void *)this);
		_call_state = CallState::PROCESS;
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
			case CallState::PROCESS:
			{
				std::cout << "thread: " << std::this_thread::get_id() << " PROCESS" << std::endl;
				new AsyncServerUnaryCall(this->_service, this->_cq, this->_engine_ptr);
				
				this->_response.set_message("Hello " + this->_request.name());
				std::this_thread::sleep_for(3s);

				_call_state = CallState::FINISH;
				_rpc.Finish(this->_response, grpc::Status::OK, (void *)this);
				break;
			}

			case CallState::FINISH:
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
	enum class CallState : int
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
	AsyncServerBidiCall(const std::shared_ptr<Greeter::AsyncService>& service, const std::shared_ptr<grpc::ServerCompletionQueue>& cq, const std::shared_ptr<engine::engine_interface>& engine_ptr)
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


grpc_server::grpc_server(const std::shared_ptr<engine::engine_interface>& engine_ptr)
: _engine_ptr{engine_ptr}
{
	spdlog::trace("creating grpc_server");

	const std::string address = "0.0.0.0";
	const std::string port = "50051";
	_server_uri = address + ":" + port;
	_num_threads_bidi_call = 1;
	_num_threads_unary_call = 4;

	// const std::string address = get_environment_variable("SERVER_ADDRESS", "0.0.0.0");
	// const std::string port = get_environment_variable("SERVER_PORT", "50051");
	// _server_uri = address + ":" + port;
	// _num_threads_bidi_call = std::stoi(get_environment_variable("NUM_THREADS_BIDI", "1"));
	// _num_threads_unary_call = std::stoi(get_environment_variable("NUM_THREADS_UNARY", "4"));
}

grpc_server::~grpc_server()
{
	spdlog::trace("deleting grpc_server");
	if(_is_running)
		stop();
}

void grpc_server::start()
{
	spdlog::info("start grpc_server");

	_service = std::make_shared<Greeter::AsyncService>();

	grpc::ServerBuilder builder;
	builder.AddListeningPort(_server_uri, grpc::InsecureServerCredentials());
	builder.RegisterService(_service.get());

	// builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 1000);
	// builder.SetDefaultCompressionLevel(GRPC_COMPRESS_LEVEL_HIGH);
	// builder.SetDefaultCompressionAlgorithm(GRPC_COMPRESS_GZIP);

	for (auto cq_idx = 0; cq_idx < 2; ++cq_idx)
		_completion_queues.emplace_back(builder.AddCompletionQueue());

	_server = builder.BuildAndStart();

	for (auto thread_idx = 0; thread_idx < _num_threads_bidi_call; ++thread_idx)
	{
		const auto cq = _completion_queues[0];
		new AsyncServerBidiCall<HelloRequest, HelloReply>(_service, cq, _engine_ptr);
		_server_threads.emplace_back([this](const auto& cq){ grpc_thread_worker(cq); }, cq);
	}

	for (auto thread_idx = 0; thread_idx < _num_threads_unary_call; ++thread_idx)
	{
		const auto cq = _completion_queues[1];
		new AsyncServerUnaryCall<HelloRequest, HelloReply>(_service, cq, _engine_ptr);
		_server_threads.emplace_back([this](const auto& cq){ grpc_thread_worker(cq); }, cq);
	}

	spdlog::info("Server running @ {}", _server_uri);
	spdlog::info("num_threads_unary_call: {}", _num_threads_unary_call);
	spdlog::info("num_threads_bidi_call: {}", _num_threads_bidi_call);

	_is_running = true;
}

void grpc_server::stop()
{
	spdlog::info("stop grpc_server");

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
}

void grpc_server::grpc_thread_worker(const std::shared_ptr<grpc::ServerCompletionQueue>& cq)
{
	while (_is_running)
	{
		void *call_tag = nullptr;
		bool ok = false;
		if (!cq->Next(&call_tag, &ok))
		{
			spdlog::warn("Server stream closed. Finish worker thread");
			break;
		}

		auto call = static_cast<AsyncServerCall*>(call_tag);
		if (!call->process(ok))
			break;
	}
}

}