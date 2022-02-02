#pragma once

#include "infer_response.hpp"
#include "infer_request.hpp"

namespace tie::backend
{
enum class type { null, onnx, pytorch };

class backend_interface
{
public:
    virtual ~backend_interface() = default;
    virtual infer_response infer(const infer_request& request) = 0;
};
}
