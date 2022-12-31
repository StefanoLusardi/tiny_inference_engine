#include "engine.hpp"

#include <backend_factory.hpp>
#include <backend_interface.hpp>

#include <spdlog/spdlog.h>

namespace tie::engine
{
engine::engine() : _backend { backend_factory::create(backend_factory::type::onnx) }
{
    spdlog::debug("creating engine");
}

engine::~engine()
{
    spdlog::debug("deleting engine");
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
    return _backend->load_models({model_name});
}

bool engine::unload_model(const std::string_view& model_name) const
{
    // _backend->unload_model(model_name);
    return true;
}

infer_response engine::infer(const infer_request& request)
{
    return _backend->infer(request);
}

}