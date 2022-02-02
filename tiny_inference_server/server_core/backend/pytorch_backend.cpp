#include "pytorch_backend.hpp"
#include <spdlog/spdlog.h>

namespace tie::backend
{
pytorch_backend::pytorch_backend() noexcept
{
}

pytorch_backend::~pytorch_backend()
{
}

infer_response pytorch_backend::infer(const infer_request& request)
{
    return {};
}

}
