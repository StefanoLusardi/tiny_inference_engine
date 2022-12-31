#pragma once

#include <CLI/CLI.hpp>

#include "server_config.hpp"


namespace tie::server
{
class command_line_parser : public CLI::App
{
public:
    explicit command_line_parser() noexcept;
    ~command_line_parser() noexcept = default;
    server_config config();
    void dump();

private:
    server_config _config;
};

}