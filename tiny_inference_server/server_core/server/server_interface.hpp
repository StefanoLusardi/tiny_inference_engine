#pragma once

namespace tie::server
{
struct server_interface
{
	virtual ~server_interface() = default;
	virtual const char*  id() const = 0;
	virtual void start() = 0;
	virtual void stop() = 0;
};
}