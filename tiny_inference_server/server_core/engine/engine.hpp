#pragma once

#include "engine_interface.hpp"

#include <memory>
#include <string_view>

namespace tie::backend
{
    class backend_interface;
}

namespace tie::engine
{
class engine final : public engine_interface
{
public:
    explicit engine();
    ~engine() override;
    bool is_ready() const override;
    bool load_model(const std::string_view& model_name) const override;
    bool unload_model(const std::string_view& model_name) const override;
    bool is_model_ready(const std::string_view& model_name) const override;
    backend::infer_response infer(const backend::infer_request& request) override;

private:
    std::unique_ptr<tie::backend::backend_interface> _backend;
};
}