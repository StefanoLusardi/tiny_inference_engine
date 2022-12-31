#include "command_line_parser.hpp"
#include <CLI/Validators.hpp>
#include <spdlog/spdlog.h>


namespace tie::server
{
command_line_parser::command_line_parser() noexcept : CLI::App("Tiny Inference Server", "v0.0.1")
{
    set_version_flag("-v,--version", std::string("0.0.1"));
    allow_config_extras();
    
    _config.restart_server = false;
    add_flag("--restart_server", _config.restart_server, "Restart server after shutdown")
        ->default_val(false)
        ->envname("TIE_RESTART_SERVER");

    add_option("--models_repo", _config.models_repo, "Models repository")
        ->default_val("models")
        ->check(CLI::ExistingDirectory)
        ->envname("TIE_MODELS_REPO");
    
    add_option("--log_level", _config.log_level, "Logger Level")
        ->group("Logger")
        ->default_val("trace")
        ->check(CLI::IsMember({"trace", "debug", "info", "warning", "error", "critical", "off"}, CLI::ignore_case, CLI::ignore_underscore))
        ->envname("TIE_LOGGER_LEVEL");

    add_option("--log_file", _config.log_file, "Logger file")
        ->group("Logger")
        ->default_val("server.log")
        ->envname("TIE_LOGGER_FILE");

    _config.is_http_server_enabled = false;
    auto http_enabled = add_flag("--enable_http_server", _config.is_http_server_enabled, "Enable HTTP Server")
        ->group("HTTP Server")
        ->default_val(false)
        ->envname("TIE_HTTP_SERVER_ENABLED");

    add_option("--http_address", _config.http_address, "http address")
        ->group("HTTP Server")
        ->default_val("0.0.0.0")
        ->check(CLI::ValidIPV4)
        ->needs(http_enabled)
        ->envname("TIE_HTTP_ADDRESS");

    add_option("--http_port", _config.http_port, "http port")
        ->group("HTTP Server")
        ->default_val("8080")
        ->check(CLI::PositiveNumber)
        ->needs(http_enabled)
        ->envname("TIE_HTTP_PORT");

    _config.is_grpc_server_enabled = true;
    auto grpc_enabled = add_flag("--enable_grpc_server", _config.is_grpc_server_enabled, "Enable gRPC Server")
        ->group("gRPC Server")
        ->default_val(true)
        ->envname("TIE_GRPC_SERVER_ENABLED");

    add_option("--grpc_address", _config.grpc_address, "gRPC address")
        ->group("gRPC Server")
        ->default_val("0.0.0.0")
        ->check(CLI::ValidIPV4)
        ->needs(grpc_enabled)
        ->envname("TIE_GRPC_ADDRESS");

    add_option("--grpc_port", _config.grpc_port, "gRPC port")
        ->group("gRPC Server")
        ->default_val("50051")
        ->check(CLI::PositiveNumber)
        ->needs(grpc_enabled)
        ->envname("TIE_GRPC_PORT");

    add_option("--grpc_inference_threads", _config.grpc_inference_threads, "gRPC number of inference threads")
        ->group("gRPC Server")
        ->default_val(4)
        ->check(CLI::PositiveNumber)
        ->needs(grpc_enabled)
        ->envname("TIE_GRPC_INFER_THREADS");
}

server_config command_line_parser::config()
{
    return _config;
}

void command_line_parser::dump()
{
    std::cout << "\n\n" << config_to_str(true, true) << "\n\n" << std::endl;
}

}