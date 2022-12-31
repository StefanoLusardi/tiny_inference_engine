#include "null_backend.hpp"
#include <spdlog/spdlog.h>

namespace tie::engine
{
null_backend::null_backend() noexcept {}

null_backend::~null_backend() {}

bool null_backend::load_models(const std::vector<std::string_view>& models)
{
    return true;
}

infer_response null_backend::infer(const infer_request& request)
{
    return {};
}

}
