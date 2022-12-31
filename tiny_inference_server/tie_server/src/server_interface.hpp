#pragma once

#include "server_config.hpp"

namespace tie::server
{
struct server_interface
{
	virtual ~server_interface() = default;
	virtual bool start(const server_config& config) = 0;
	virtual void stop() = 0;
};
}