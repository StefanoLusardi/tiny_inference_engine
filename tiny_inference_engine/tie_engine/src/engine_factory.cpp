#include <tie_engine/engine_factory.hpp>
#include "engine.hpp"

#include <spdlog/spdlog.h>

namespace tie::engine
{
std::unique_ptr<engine_interface> engine_factory::create()
{
    return std::make_unique<engine>();
}

}