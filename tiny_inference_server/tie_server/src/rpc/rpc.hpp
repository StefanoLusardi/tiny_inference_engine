#pragma once

#include <cstdint>
#include <string>

namespace tie::server
{
class rpc
{
public:
    explicit rpc(const char* name, uint64_t id) : name { name }, id { id } { }
    virtual ~rpc() = default;
    virtual bool execute() = 0;

    const std::string name;
    const uint64_t id = 0;
};

}
