#include "engine.hpp"

#include <spdlog/spdlog.h>

namespace xyz::engine
{
engine::engine()
{
    spdlog::trace("creating engine");
}

engine::~engine()
{
    spdlog::trace("deleting engine");
}

}