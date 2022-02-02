#pragma once

#include "backend_interface.hpp"

#include <memory>

namespace tie::backend
{
class pytorch_backend : public backend_interface
{
public:
    explicit pytorch_backend() noexcept;
    ~pytorch_backend();
    infer_response infer(const infer_request& request) override;

private:
};

}
