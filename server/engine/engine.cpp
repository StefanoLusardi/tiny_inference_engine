#include "engine.hpp"

#include "../backend/backend_interface.hpp"
#include "../backend/onnx_backend.hpp"

#include <spdlog/spdlog.h>

namespace xyz::engine
{
engine::engine() : _backend { std::make_unique<xyz::backend::onnx_backend>()}
{
    spdlog::trace("creating engine");
}

engine::~engine()
{
    spdlog::trace("deleting engine");
}

}