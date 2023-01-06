#include "server_config.hpp"
#include "server_manager.hpp"
#include "server_shutdown.hpp"
#include "command_line_parser.hpp"
#include "logging.hpp"

int main(int argc, char** argv)
{
    tie::server::install_shutdown_signal_handlers();
    tie::server::command_line_parser command_line_parser;

    try
    {
        command_line_parser.parse(argc, argv);
    }
    catch (const CLI::ParseError& e)
    {
        return command_line_parser.exit(e);
    }
    
    const tie::server::server_config config = command_line_parser.config();
    tie::server::setup_loggers(config);
    command_line_parser.dump();

	bool restart_server = true;
    while (restart_server)
    {
        tie::server::server_manager server(config);
        restart_server = server.run();
        tie::server::reset_shutdown_signal_handlers();
    }

    spdlog::info("shutdown");
    return 0;
}
