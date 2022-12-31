#pragma once

#include <memory>
#include <thread>

#include "server_interface.hpp"
#include "server_config.hpp"

namespace tie::engine
{
    class engine_interface;
}

namespace tie::server
{
class server_manager
{
public:
    explicit server_manager(const server_config& config) noexcept;
    ~server_manager() noexcept;
    bool run();

private:
    void main_loop();
    const server_config _config;
    std::shared_ptr<tie::engine::engine_interface> _engine;
    
    std::unique_ptr<tie::server::server_interface> _grpc_server;
    void start_grpc();
    void stop_grpc();

    std::unique_ptr<tie::server::server_interface> _http_server;
    void start_http();
    void stop_http();

};

}