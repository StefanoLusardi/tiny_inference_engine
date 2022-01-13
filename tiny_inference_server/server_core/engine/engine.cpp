#include "engine.hpp"

#include "../backend/backend_interface.hpp"
#include "../backend/onnx_backend.hpp"

#include <spdlog/spdlog.h>

namespace tie::engine
{
engine::engine() : _backend { std::make_unique<tie::backend::onnx_backend>()}
{
    spdlog::trace("creating engine");
}

engine::~engine()
{
    spdlog::trace("deleting engine");
}

bool engine::is_ready() const
{
    return true;
}

bool engine::is_model_ready(const std::string_view& model_name) const
{
    // _backend->is_model_ready(model_name);
    return true;
}

bool engine::load_model(const std::string_view& model_name) const
{
    // _backend->load_model(model_name);
    return true;
}

bool engine::unload_model(const std::string_view& model_name) const
{
    // _backend->unload_model(model_name);
    return true;
}

backend::infer_response engine::infer(const backend::infer_request& request)
{
    return _backend->infer(request);
}

}