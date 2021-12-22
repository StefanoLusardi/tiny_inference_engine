#pragma once

namespace xyz::engine
{
struct server_interface
{
	virtual ~server_interface() = default;
	virtual void start() = 0;
	virtual void stop() = 0;
};
}