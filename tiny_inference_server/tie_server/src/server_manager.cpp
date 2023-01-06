#include "server_manager.hpp"
#include "grpc_server.hpp"
#include "http_server.hpp"
#include "server_shutdown.hpp"

#include <tie_engine/engine_factory.hpp>
#include <spdlog/spdlog.h>

namespace tie::server
{
server_manager::server_manager(const server_config& config) noexcept 
: _config { config } 
, _engine { tie::engine::engine_factory::create() }
{
    spdlog::debug("create server manager");
}

server_manager::~server_manager() noexcept
{
    spdlog::debug("delete server manager");
    stop_grpc();
    stop_http();
}

bool server_manager::run()
{
    bool restart_server = _config.restart_server;
    
    start_grpc();
    start_http();

    try
    {
        main_loop();
    }
    catch(const std::exception& e)
    {
        spdlog::error("unhandled error: {}", e.what());
    }

    return restart_server;
}

void server_manager::main_loop()
{
    unsigned tick = 0;
    while (!shutdown_server())
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
        spdlog::trace(tick++);
    }
}

void server_manager::start_grpc()
{
    if(!_config.is_grpc_server_enabled)
        return;

    spdlog::debug("start grpc server");
    _grpc_server = std::make_unique<tie::server::grpc_server>(_engine);
    _grpc_server->start(_config);
}

void server_manager::start_http()
{
    if(!_config.is_http_server_enabled)
        return;

    spdlog::debug("start http server");
    _http_server = std::make_unique<tie::server::http_server>(_engine);
    _http_server->start(_config);
}

void server_manager::stop_grpc()
{
    if(!_config.is_grpc_server_enabled)
        return;

    spdlog::debug("stop grpc server");
    _grpc_server->stop();
}

void server_manager::stop_http()
{
    if(!_config.is_http_server_enabled)
        return;

    spdlog::debug("stop http server");
    _http_server->stop();
}

}
