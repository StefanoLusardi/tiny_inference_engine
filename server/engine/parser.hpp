#pragma once

#include <memory>
#include <string>
#include "config.hpp"

namespace CLI { class App; }

namespace xyz
{
class parser
{
public:
    explicit parser() noexcept;
    ~parser() noexcept;
    
    auto parse_cli(int argc, char** argv) -> std::tuple<bool, int>;
    void dump() const;
    config get_config() {return _config; }
    std::string log_level() { return _config.log_level; }
    bool is_grpc_server_enabled() const { return _config.grpc_server_enabled; }
    bool is_http_server_enabled() const { return _config.http_server_enabled; }

private:
    std::unique_ptr<CLI::App> _cli;
    config _config;
};
}