#include "null_backend.hpp"
#include <spdlog/spdlog.h>

namespace tie::engine
{
null_backend::null_backend() noexcept {}

null_backend::~null_backend() {}

auto null_backend::is_model_ready(const std::string& model_name, const std::string& model_version) const -> bool
{
    return true;
}

auto null_backend::model_load(const std::string& model_name, const std::string& model_version) -> bool
{
    return true;
}

auto null_backend::model_unload(const std::string& model_name, const std::string& model_version) -> bool
{
    return true;
}

auto null_backend::model_metadata(const std::string& model_name, const std::string& model_version) -> common::model_metadata
{
    return {};
}

auto null_backend::infer(const infer_request& request) -> infer_response
{
    return {};
}

}
