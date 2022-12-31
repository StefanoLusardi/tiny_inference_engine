#pragma once

#include <backend_interface.hpp>

#include <memory>

namespace tie::engine
{
class null_backend : public backend_interface
{
public:
    explicit null_backend() noexcept;
    ~null_backend();
    bool load_models(const std::vector<std::string_view>& models) override;
    infer_response infer(const infer_request& request) override;

private:
};

}
