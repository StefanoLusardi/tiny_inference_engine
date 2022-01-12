#pragma once

#include <string>

namespace xyz
{
struct config
{
    std::string log_level;
    
    bool http_server_enabled;
    std::string http_address;
    std::string http_port;

    bool grpc_server_enabled;
    std::string grpc_address;
    std::string grpc_port;
    int grpc_threads;
};
}