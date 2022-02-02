#pragma once

#include "backend_interface.hpp"

#include <memory>

namespace tie::backend
{
class null_backend : public backend_interface
{
public:
    explicit null_backend() noexcept;
    ~null_backend();
    infer_response infer(const infer_request& request) override;

private:
};

}
