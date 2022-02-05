#include "pytorch_backend.hpp"
#include <spdlog/spdlog.h>

namespace tie::backend
{
pytorch_backend::pytorch_backend() noexcept {}

pytorch_backend::~pytorch_backend() {}

bool pytorch_backend::load_models(const std::vector<std::string_view>& models)
{
    return true;
}

infer_response pytorch_backend::infer(const infer_request& request)
{
    return {};
}

}
