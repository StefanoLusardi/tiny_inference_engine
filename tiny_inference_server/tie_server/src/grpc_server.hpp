#pragma once

#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <string>

#include <services.grpc.pb.h>

#include "server_interface.hpp"
#include <tie_engine/engine_interface.hpp>


namespace tie::server
{
class grpc_server final : public tie::server::server_interface
{
public:
	explicit grpc_server(const std::shared_ptr<engine::engine_interface>& engine);
	~grpc_server();

	bool start(const server_config& config) override;
	void stop() override;

private:
	void rpc_handler(const std::shared_ptr<grpc::ServerCompletionQueue>& cq);
	std::shared_ptr<engine::engine_interface> _engine;
	std::atomic_bool _is_running;

	std::vector<std::thread> _server_threads;

	std::vector<std::shared_ptr<grpc::ServerCompletionQueue>> _rpc_inference_queues;
	std::shared_ptr<grpc::ServerCompletionQueue> _rpc_common_queue;

	std::shared_ptr<inference::GRPCInferenceService::AsyncService> _service;
	std::unique_ptr<grpc::Server> _server;
};

}
