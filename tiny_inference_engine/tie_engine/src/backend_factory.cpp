#include <backend_factory.hpp>
#include <backend_interface.hpp>
#include "null_backend/null_backend.hpp"

#include <spdlog/spdlog.h>

namespace tie::engine
{
std::unique_ptr<backend_interface> backend_factory::create(const backend_factory::type backend_type)
{
    switch (backend_type)
    {
        case type::null: return std::make_unique<null_backend>();
        // case type::onnx: return std::make_unique<onnx_backend>>();
        default:
            return nullptr;
    }

    return {};
}

}