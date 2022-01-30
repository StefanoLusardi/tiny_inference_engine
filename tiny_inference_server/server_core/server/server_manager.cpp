#include "server_manager.hpp"
#include "grpc_server.hpp"
#include "http_server.hpp"

#include <spdlog/spdlog.h>

namespace tie::server
{

server_manager::server_manager(const std::shared_ptr<tie::engine::engine_interface>& engine_ptr)
    : _engine_ptr{engine_ptr}
{
    spdlog::trace("created server manager");
}

void server_manager::run(const server_type server_type)
{
    try 
    {
        switch (server_type)
        {
            case server_type::grpc:
            {
                _grpc_server = std::make_unique<tie::server::grpc_server>(_engine_ptr);
                _grpc_thread = std::make_unique<std::thread>([this]{ _grpc_server->start(); });
                spdlog::trace("created grpc server");
                break;
            }
            case server_type::http:
            {
                _http_server = std::make_unique<tie::server::http_server>(_engine_ptr);
                _http_thread = std::make_unique<std::thread>([this]{ _http_server->start(); });
                spdlog::trace("created http server");
                break;
            }
        }
    }
    catch (std::exception e)
    {
        spdlog::error("error running server manager: {}", e.what());
    }
}

server_manager::~server_manager()
{
    spdlog::trace("deleting server manager");

    if(_http_server)
    {
        spdlog::trace("deleting http server");
        _http_server->stop();
        if (_http_thread->joinable())
            _http_thread->join();
    }

    if(_grpc_server)
    {
        spdlog::trace("deleting grpc server");
        _grpc_server->stop();
        if (_grpc_thread->joinable())
            _grpc_thread->join();
    }
}

}
