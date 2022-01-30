#include "http_server.hpp"
#include <spdlog/spdlog.h>

namespace tie::server
{
http_server::http_server(const std::shared_ptr<engine::engine_interface>& engine_ptr)
    : _engine_ptr{engine_ptr}
    , _is_running {false}
{
}

http_server::~http_server() { }
}