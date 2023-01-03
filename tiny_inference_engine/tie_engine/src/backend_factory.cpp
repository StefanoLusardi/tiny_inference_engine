#include <tie_engine/backend_factory.hpp>
#include "null_backend/null_backend.hpp"

#include <spdlog/spdlog.h>

namespace tie::engine
{
std::unique_ptr<backend_interface> backend_factory::create(backend_type backend_type)
{
    switch (backend_type)
    {
        case backend_type::null: 
            return std::make_unique<null_backend>();

        // case backend_type::onnx: 
        //     return std::make_unique<onnx_backend>>();

        default:
            return nullptr;
    }

    return {};
}

}