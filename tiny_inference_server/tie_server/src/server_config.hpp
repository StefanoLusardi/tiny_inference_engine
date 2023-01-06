#pragma once

#include <filesystem>
#include <string>

namespace tie::server
{
struct server_config
{
    bool restart_server;
    std::filesystem::path models_repo;

    // Logger
    bool enable_console_logger;
    std::string console_logger_level;
    bool enable_file_logger;
    std::string file_logger_level;
    std::filesystem::path log_file;
    
    // HTTP Server
    bool is_http_server_enabled;
    std::string http_address;
    std::string http_port;

    // gRPC Server
    bool is_grpc_server_enabled;
    std::string grpc_address;
    std::string grpc_port;
    int grpc_inference_threads;
};
}