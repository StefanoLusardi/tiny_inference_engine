#pragma once

#include <memory>
#include <atomic>

#include "server_interface.hpp"
#include "../engine/engine_interface.hpp"

namespace tie::server
{
class http_server final : public tie::server::server_interface
{
public:
	explicit http_server(const std::shared_ptr<engine::engine_interface>& engine_ptr);
	~http_server();

	const char* id() const override { return "http_server";}
	void start() override { }
	void stop() override { }

private:
	std::shared_ptr<engine::engine_interface> _engine_ptr;
	std::atomic_bool _is_running;
};
}