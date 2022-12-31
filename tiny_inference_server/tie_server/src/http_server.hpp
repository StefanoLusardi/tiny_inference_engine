#pragma once

#include <memory>
#include <atomic>

#include "server_interface.hpp"
#include <engine_interface.hpp>

namespace tie::server
{
class http_server final : public tie::server::server_interface
{
public:
	explicit http_server(const std::shared_ptr<engine::engine_interface>& engine);
	~http_server();

	bool start(const server_config& config) override { (void)config; return true; }
	void stop() override { }

private:
	std::shared_ptr<engine::engine_interface> _engine;
	std::atomic_bool _is_running;
};
}