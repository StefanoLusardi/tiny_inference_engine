#include "command_line_parser.hpp"
#include "version.hpp"
#include <CLI/Validators.hpp>
#include <spdlog/spdlog.h>


namespace tie::server
{
command_line_parser::command_line_parser() noexcept : CLI::App(TIE_SERVER_NAME, TIE_SERVER_VERSION)
{
    set_version_flag("-v,--version", TIE_SERVER_VERSION);
    allow_config_extras();
    
    // Restart Server
    _config.restart_server = false;
    add_flag("--restart_server", _config.restart_server, "Restart server after shutdown")
        ->default_val(false)
        ->envname("TIE_RESTART_SERVER");

    // Models Repository
    add_option("--models_repo", _config.models_repo, "Models repository")
        ->default_val("models")
        ->check(CLI::ExistingDirectory)
        ->envname("TIE_MODELS_REPO");
    
    // Logger - Console
    _config.enable_console_logger = true;
    auto console_logger_enabled = add_flag("--enable_console_logger", _config.enable_console_logger, "Enable console logger")
        ->group("Logger")
        ->default_val(true)
        ->envname("TIE_ENABLE_CONSOLE_LOGGER");
    
    add_option("--console_logger_level", _config.console_logger_level, "Console logger Level")
        ->group("Logger")
        ->needs(console_logger_enabled)
        ->default_val("trace")
        ->check(CLI::IsMember({"trace", "debug", "info", "warning", "error", "critical", "off"}, CLI::ignore_case, CLI::ignore_underscore))
        ->envname("TIE_CONSOLE_LOGGER_LEVEL");

    // Logger - File
    _config.enable_file_logger = true;
    auto file_logger_enabled = add_flag("--enable_file_logger", _config.enable_file_logger, "Enable file logger")
        ->group("Logger")
        ->default_val(false)
        ->envname("TIE_ENABLE_FILE_LOGGER");
    
    add_option("--file_logger_level", _config.file_logger_level, "File logger Level")
        ->group("Logger")
        ->default_val("trace")
        ->needs(file_logger_enabled)        
        ->check(CLI::IsMember({"trace", "debug", "info", "warning", "error", "critical", "off"}, CLI::ignore_case, CLI::ignore_underscore))
        ->envname("TIE_FILE_LOGGER_LEVEL");

    add_option("--log_file", _config.log_file, "Logger file")
        ->group("Logger")
        ->default_val("server.log")
        ->needs(file_logger_enabled)
        ->envname("TIE_LOGGER_FILE");

    // Server HTTP
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

    // Server gRPC
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
    spdlog::trace("\n{}\n", TIE_BANNER);
    spdlog::info("\n{}\n", config_to_str(true, true));
}

}