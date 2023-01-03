#pragma once

#include <tie_engine/engine_interface.hpp>
#include <memory>
#include <string>
#include <vector>

namespace tie::engine
{
class backend_interface;

class engine final : public engine_interface
{
public:
    explicit engine();
    ~engine() override;
    
    bool is_engine_ready() const override;
    auto model_list() const -> std::vector<std::string> override;
    auto is_model_ready(const std::string& model_name, const std::string& model_version) const -> bool override;
    auto model_load(const std::string& model_name, const std::string& model_version) const -> bool override;
    auto model_unload(const std::string& model_name, const std::string& model_version) const -> bool override;
    auto model_metadata(const std::string& model_name, const std::string& model_version) const -> common::model_metadata override;
    auto infer(const infer_request& infer_request) -> infer_response override;

private:
    std::unique_ptr<backend_interface> _backend;
};
}