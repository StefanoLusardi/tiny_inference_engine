#pragma once

#include <tie_engine/infer_request.hpp>
#include <tie_engine/infer_response.hpp>
#include <tie_engine/model_metadata.hpp>

namespace tie::engine
{
class backend_interface
{
public:
    virtual ~backend_interface() = default;

    virtual auto is_model_ready(const std::string& model_name, const std::string& model_version) const -> bool = 0;
    virtual auto model_load(const std::string& model_name, const std::string& model_version) -> bool = 0;
    virtual auto model_unload(const std::string& model_name, const std::string& model_version) -> bool = 0;
    virtual auto model_metadata(const std::string& model_name, const std::string& model_version) -> common::model_metadata = 0;
    virtual auto infer(const infer_request& infer_request) -> infer_response = 0;
};
}
