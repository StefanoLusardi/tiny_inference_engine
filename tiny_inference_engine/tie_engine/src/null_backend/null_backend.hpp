#pragma once

#include <tie_engine/backend_interface.hpp>

#include <memory>

namespace tie::engine
{
class null_backend : public backend_interface
{
public:
    explicit null_backend() noexcept;
    ~null_backend();

    auto is_model_ready(const std::string& model_name, const std::string& model_version) const -> bool override;
    auto model_load(const std::string& model_name, const std::string& model_version) -> bool override;
    auto model_unload(const std::string& model_name, const std::string& model_version) -> bool override;
    auto model_metadata(const std::string& model_name, const std::string& model_version) -> common::model_metadata override;
    auto infer(const infer_request& infer_request) -> infer_response override;
};

}
