#include "parser.hpp"
#include <thread>
#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

namespace tie
{
parser::parser() noexcept : _cli{std::make_unique<CLI::App>("xyz inference engine")}
{
    spdlog::trace("creating parser");

    _cli->set_version_flag("-v,--version", std::string("xyz inference server v0.0.1"));

    _config.log_level = "trace";
    _cli->add_option("-l,--log_level", _config.log_level, "Logger Level")
        ->envname("XYZ_LOGGER_LEVEL");

    _config.http_server_enabled = false;
    auto http_enabled = _cli->add_option("--enable_http_server", _config.http_server_enabled, "Enable http server")
        ->envname("XYZ_HTTP_SERVER_ENABLED");

    _config.http_address = "0.0.0.0";
    _cli->add_option("--http_address", _config.http_address, "http address")
        ->needs(http_enabled)
        ->envname("XYZ_HTTP_ADDRESS");

    _config.http_port = "50051";
    _cli->add_option("--http_port", _config.http_port, "http port")
        ->needs(http_enabled)
        ->envname("XYZ_HTTP_PORT");

    _config.grpc_server_enabled = true;
    auto grpc_enabled = _cli->add_option("--enable_grpc_server", _config.grpc_server_enabled, "Enable gRPC server")
        ->envname("XYZ_GRPC_SERVER_ENABLED");

    _config.grpc_address = "0.0.0.0";
    _cli->add_option("--grpc_address", _config.grpc_address, "gRPC address")
        ->needs(grpc_enabled)
        ->envname("XYZ_GRPC_ADDRESS");

    _config.grpc_port = "50051";
    _cli->add_option("--grpc_port", _config.grpc_port, "gRPC port")
        ->needs(grpc_enabled)
        ->envname("XYZ_GRPC_PORT");

    _config.grpc_threads = 4;
    _cli->add_option("--grpc_threads", _config.grpc_threads, "Number of gRPC threads")
        ->needs(grpc_enabled)
        ->envname("XYZ_GRPC_THREADS")
        ->expected(1, std::thread::hardware_concurrency());
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