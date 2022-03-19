#include <exception>
#include <thread>
#include <chrono>

#include "include/run.hpp"
#include "shutdown.hpp"
#include "parser.hpp"
#include "config.hpp"
#include "engine.hpp"

#include "../server/server_manager.hpp"

#include <spdlog/spdlog.h>

namespace tie::engine
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

        auto engine_ptr = std::make_shared<engine>();
        auto server_manager = tie::server::server_manager(engine_ptr);

        if(cli.is_grpc_server_enabled())
            server_manager.run(tie::server::server_type::grpc); //, cli.get_config());
        
        if(cli.is_http_server_enabled())
            server_manager.run(tie::server::server_type::http); //,cli.get_config());

        unsigned int t = 0;
        // while (!shutdown_engine())
        while (t<3)
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

    spdlog::info("shutting down tie infernce server");
    return EXIT_SUCCESS;
}
}