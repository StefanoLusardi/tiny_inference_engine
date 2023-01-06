#include "engine.hpp"
#include <tie_engine/backend_factory.hpp>
#include <tie_engine/backend_interface.hpp>

#include <spdlog/spdlog.h>

namespace tie::engine
{
engine::engine() 
: _backend { backend_factory::create(backend_type::null) }
{
    spdlog::debug("creating engine");
}

engine::~engine()
{
    spdlog::debug("deleting engine");
}

bool engine::is_engine_ready() const
{
    // TODO: check that all models are ready
    return true;
}

auto engine::model_list() const -> std::vector<std::string>
{
    return {};
}

auto engine::is_model_ready(const std::string& model_name, const std::string& model_version) const -> bool
{
    return _backend->is_model_ready(model_name, model_version);
}

auto engine::model_load(const std::string& model_name, const std::string& model_version) const -> bool
{
    spdlog::info("engine::model_load");
    return _backend->model_load(model_name, model_version);
}

auto engine::model_unload(const std::string& model_name, const std::string& model_version) const -> bool
{
    return _backend->model_unload(model_name, model_version);
}

auto engine::model_metadata(const std::string& model_name, const std::string& model_version) const -> common::model_metadata
{
    return _backend->model_metadata(model_name, model_version);
}

auto engine::infer(const infer_request& request) -> infer_response
{
    return _backend->infer(request);
}

}