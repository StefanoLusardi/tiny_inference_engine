#pragma once

#include <memory>
#include <exception>
#include <thread>

#include "config.hpp"
#include "server_interface.hpp"
#include "engine_interface.hpp"

#include <spdlog/spdlog.h>

namespace xyz::engine
{
template<class ServerT>
class server_manager
{
public:
    explicit server_manager()
    {
        static_assert(std::is_base_of_v<server_interface, ServerT>, "ServerT must derive from xyz::engine::server_interface");
    }

    void run(const std::shared_ptr<engine_interface>& engine_ptr, config config)
    {
        try 
        {
            _server = std::make_unique<ServerT>(engine_ptr);
            spdlog::trace("running server manager: {}", _server->id());
            _server_thread = std::thread([this]{ _server->start(); });
        }
        catch (std::exception e)
        {
            spdlog::error("error creating server manager: {} - {}", _server->id(), e.what());
        }
    }

    ~server_manager()
    {
        if(!_server)
            return;

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