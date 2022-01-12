#pragma once

#include <thread>

#include <grpcpp/grpcpp.h>
#include <services.grpc.pb.h>

#include "server_interface.hpp"
#include "../engine/engine_interface.hpp"

namespace xyz::server
{
class grpc_server final : public engine::server_interface
{
public:
	explicit grpc_server(const std::shared_ptr<engine::engine_interface>& engine_ptr);
	~grpc_server();

	const char* id() const override { return "grpc_server";}
	void start() override;
	void stop() override;

private:
	void grpc_thread_worker(const std::shared_ptr<grpc::ServerCompletionQueue>& cq);

	std::shared_ptr<engine::engine_interface> _engine_ptr;
	std::atomic_bool _is_running;
	std::vector<std::thread> _server_threads;
	std::map<std::string_view, std::shared_ptr<grpc::ServerCompletionQueue>> _completion_queues;
	std::shared_ptr<tie::InferenceService::AsyncService> _service;
	std::unique_ptr<grpc::Server> _server;
	
	std::string _server_uri;
	unsigned int _num_threads_inference_single;
	unsigned int _num_threads_inference_multi;
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