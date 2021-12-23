#pragma once

#include <thread>
#include "config.hpp"
#include "server_interface.hpp"
#include <spdlog/spdlog.h>

namespace xyz::engine
{
template<class ServerT>
class server_manager
{
public:
    explicit server_manager()
    {
        static_assert(std::is_base_of<server_interface, ServerT>::value, "ServerT must derive from xyz::engine::server_interface");
    }

    void run(config config)
    {
        spdlog::trace("running server manager: {}", _server->id());
        _server = std::make_unique<ServerT>();
        _server_thread = std::thread([this]{ _server->start(); });
    }

    ~server_manager()
    {
        spdlog::trace("deleting server manager: {}", _server->id());

        _server->stop();
        if (_server_thread.joinable())
            _server_thread.join();
    }

private:
    std::unique_ptr<server_interface> _server;
    std::thread _server_thread;
};

}