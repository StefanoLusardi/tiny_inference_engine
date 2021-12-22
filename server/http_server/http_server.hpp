#pragma once

#include "../engine/server_interface.hpp"

namespace xyz
{
class http_server final : public engine::server_interface
{
public:
	explicit http_server() { }
	~http_server() { }

	void start() override { }
	void stop() override { }
};
}