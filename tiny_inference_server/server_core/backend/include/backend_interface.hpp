#pragma once

#include "infer_response.hpp"
#include "infer_request.hpp"

namespace tie::backend
{
enum class type
{
    null,
    onnx,
    pytorch
};

class backend_interface
{
public:
    virtual ~backend_interface() = default;
    virtual bool load_models(const std::vector<std::string_view>& models) = 0;
    virtual infer_response infer(const infer_request& request) = 0;
};
}
