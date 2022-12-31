#pragma once

#include <string>

namespace tie::server
{
struct server_config
{
    bool restart_server;
    std::string models_repo;

    // Logger
    std::string log_level;
    std::string log_file;
    
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