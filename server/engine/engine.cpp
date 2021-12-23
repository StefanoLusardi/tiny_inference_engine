#include "shutdown.hpp"
#include "parser.hpp"
#include "config.hpp"
#include "server_manager.hpp"

#include "../grpc_server/grpc_server.hpp"
#include "../http_server/http_server.hpp"

#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>

namespace xyz::engine
{
using namespace std::chrono_literals;

int run(int argc, char** argv)
{
    install_shutdown_signal_handlers();

    {
        parser cli;
        if (auto[parse_ok, error_code] = cli.parse_cli(argc, argv); !parse_ok)
            return error_code; 
        
        spdlog::set_level(spdlog::level::from_str(cli.log_level()));
        cli.dump();

        server_manager<grpc_server> grpc_server_manager;
        grpc_server_manager.run(cli.get_config());

        server_manager<http_server> http_server_manager;
        http_server_manager.run(cli.get_config());

        uint t = 0;
        while (!shutdown_engine())
        {
            // engine loop
            std::this_thread::sleep_for(1s);
            spdlog::trace(t++);
        }
    }

    spdlog::info("shutting down xyz infernce engine");
    return EXIT_SUCCESS;
}
}