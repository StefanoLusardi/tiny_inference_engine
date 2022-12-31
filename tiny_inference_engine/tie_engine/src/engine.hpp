#pragma once

#include "engine_interface.hpp"

#include <memory>
#include <string_view>

namespace tie::engine
{
class backend_interface;

class engine final : public engine_interface
{
public:
    explicit engine();
    ~engine() override;
    bool is_ready() const override;
    bool load_model(const std::string_view& model_name) const override;
    bool unload_model(const std::string_view& model_name) const override;
    bool is_model_ready(const std::string_view& model_name) const override;
    infer_response infer(const infer_request& request) override;

private:
    std::unique_ptr<backend_interface> _backend;
};
}