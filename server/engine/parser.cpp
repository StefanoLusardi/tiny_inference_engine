#include "parser.hpp"
#include <thread>
#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

namespace xyz
{
parser::parser() noexcept : _cli{std::make_unique<CLI::App>("xyz inference engine")}
{
    _cli->set_version_flag("-v,--version", std::string("xyz inference server v0.0.1"));

    _config.log_level = "trace";
    _cli->add_option("-l,--log_level", _config.log_level, "Logger Level")
        ->envname("XYZ_LOGGER_LEVEL");

    _config.grpc_threads = 4;
    _cli->add_option("-t,--grpc_threads", _config.grpc_threads, "Number of gRPC threads")
        ->expected(1, std::thread::hardware_concurrency())
        ->envname("XYZ_GRPC_THREADS");

    _config.grpc_address = "0.0.0.0";
    _cli->add_option("--grpc_address", _config.grpc_address, "gRPC address")
        ->envname("XYZ_GRPC_ADDRESS");

    _config.grpc_port = "50051";
    _cli->add_option("--grpc_port", _config.grpc_port, "gRPC port")
        ->envname("XYZ_GRPC_PORT");
}

parser::~parser() noexcept
{
    spdlog::trace("deleting parser");
}

auto parser::parse_cli(int argc, char** argv) -> std::tuple<bool, int>
{
    try
    {
        _cli->parse(argc, argv);
    } 
    catch (const CLI::ParseError &e)
    {
        return std::make_tuple(false, _cli->exit(e));
    }
    return std::make_tuple(true, 0);
}

void parser::dump() const
{
    spdlog::debug("--- Configuration ---");
    spdlog::debug("log level: ", _config.log_level);
    spdlog::debug("grpc threads: ", _config.grpc_threads);
}

}