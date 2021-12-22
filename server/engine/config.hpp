#pragma once

#include <string>

namespace xyz
{
struct config
{
    std::string log_level;
    
    int grpc_threads;
    std::string grpc_address;
    std::string grpc_port;
};
}