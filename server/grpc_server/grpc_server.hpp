#pragma once

#include <thread>
#include <condition_variable>

#include <grpcpp/grpcpp.h>
#include <services.grpc.pb.h>

#include "../engine/server_interface.hpp"

using helloworld::Greeter;

namespace xyz
{
class grpc_server final : public engine::server_interface
{
public:
	explicit grpc_server();
	~grpc_server();

	const char* id() const override { return "grpc_server";}
	void start() override;
	void stop() override;

private:
	void grpc_thread_worker(const std::shared_ptr<grpc::ServerCompletionQueue>& cq);

	std::atomic_bool _is_running;
	std::vector<std::thread> _server_threads;
	std::vector<std::shared_ptr<grpc::ServerCompletionQueue>> _completion_queues;
	std::shared_ptr<Greeter::AsyncService> _service;
	std::unique_ptr<grpc::Server> _server;

	std::string _server_uri;
	unsigned int _num_threads_unary_call;
	unsigned int _num_threads_bidi_call;
};

}

/*

	grpc_server s;
	s.start();

	std::thread stop_thread([&s]
	{
		std::this_thread::sleep_for(3s);
		s.stop();
	});

	s.run();
	stop_thread.join();

*/