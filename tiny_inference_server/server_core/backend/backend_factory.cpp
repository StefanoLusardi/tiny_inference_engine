#include "backend_factory.hpp"
#include "null_backend.hpp"
#include "onnx_backend.hpp"
#include "pytorch_backend.hpp"

#include <spdlog/spdlog.h>

namespace tie::backend
{
std::unique_ptr<tie::backend::backend_interface> backend_factory::create(const tie::backend::type backend_type)
{
    switch (backend_type)
    {
        case type::null: return std::make_unique<tie::backend::null_backend>();
        case type::onnx: return std::make_unique<tie::backend::onnx_backend>();
        case type::pytorch: return std::make_unique<tie::backend::pytorch_backend>();
    }

    return {};
}

}