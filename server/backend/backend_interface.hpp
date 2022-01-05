#pragma once

#include "infer_respose.hpp"
#include "infer_request.hpp"

namespace xyz::backend
{
class backend_interface
{
public:
    virtual ~backend_interface() = default;
    virtual infer_response infer(const infer_request& request) = 0;
};
}
