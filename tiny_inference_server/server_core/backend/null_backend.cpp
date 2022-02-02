#include "null_backend.hpp"
#include <spdlog/spdlog.h>

namespace tie::backend
{
null_backend::null_backend() noexcept
{
}

null_backend::~null_backend()
{
}

infer_response null_backend::infer(const infer_request& request)
{
    return {};
}

}
