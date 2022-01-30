#pragma once

#include <memory>
#include <thread>

#include "server_interface.hpp"
#include "../engine/engine_interface.hpp"


namespace tie::server
{
enum class server_type { grpc, http };

class server_manager
{
public:
    explicit server_manager(const std::shared_ptr<tie::engine::engine_interface>& engine_ptr);
    ~server_manager();

    void run(const server_type server_type);

private:
    std::shared_ptr<tie::engine::engine_interface> _engine_ptr;

    std::unique_ptr<std::thread> _grpc_thread;
    std::unique_ptr<tie::server::server_interface> _grpc_server;

    std::unique_ptr<std::thread> _http_thread;
    std::unique_ptr<tie::server::server_interface> _http_server;
};

}