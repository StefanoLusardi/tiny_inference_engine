#include <exception>
#include <thread>
#include <chrono>

#include "include/run.hpp"
#include "shutdown.hpp"
#include "parser.hpp"
#include "config.hpp"
#include "server_manager.hpp"
#include "engine.hpp"

#include "../server/grpc_server.hpp"
#include "../server/http_server.hpp"

#include <spdlog/spdlog.h>

namespace xyz::engine
{
using namespace std::chrono_literals;

int run(int argc, char** argv)
{
    install_shutdown_signal_handlers();

    try
    {    
        parser cli;
        if (auto[parse_ok, error_code] = cli.parse_cli(argc, argv); !parse_ok)
            return error_code;
        
        spdlog::set_level(spdlog::level::from_str(cli.log_level()));
        cli.dump();

        std::shared_ptr<engine_interface> engine_ptr = std::make_shared<engine>();

        server_manager<xyz::server::grpc_server> grpc_server_manager;
        if(cli.is_grpc_server_enabled())
            grpc_server_manager.run(engine_ptr, cli.get_config());

        server_manager<xyz::server::http_server> http_server_manager;
        if(cli.is_http_server_enabled())
            http_server_manager.run(engine_ptr, cli.get_config());

        unsigned int t = 0;
        while (!shutdown_engine())
        {
            // engine loop
            std::this_thread::sleep_for(1s);
            spdlog::trace(t++);
        }
    }
    catch(std::exception e)
    {
        spdlog::error("unhandled error: {}", e.what());
    }

    spdlog::info("shutting down xyz infernce engine");
    return EXIT_SUCCESS;
}
}