#include "server_manager.hpp"
#include "server_shutdown.hpp"
#include "command_line_parser.hpp"

int main(int argc, char** argv)
{
    tie::server::install_shutdown_signal_handlers();
    tie::server::command_line_parser command_line_parser;

    try
    {
        command_line_parser.parse(argc, argv);
        command_line_parser.dump();
    }
    catch (const CLI::ParseError& e)
    {
        return command_line_parser.exit(e);
    }

	bool restart_server = true;
    while (restart_server)
    {
        tie::server::server_manager server(command_line_parser.config());
        restart_server = server.run();
        tie::server::reset_shutdown_signal_handlers();
    }

    std::cout << " shutdown" << std::endl;

    return 0;
}
