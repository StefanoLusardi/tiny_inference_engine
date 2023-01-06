#pragma once

#include <tie_client/call_result.hpp>
#include <tie_client/infer_request.hpp>
#include <tie_client/infer_response.hpp>
#include <tie_client/server_metadata.hpp>
#include <tie_client/model_metadata.hpp>

#include <memory>
#include <vector>
#include <string>

namespace tie::client
{
class client_interface
{
public:
    virtual ~client_interface() = default;

    virtual auto is_server_live() -> std::tuple<call_result, bool> = 0;
    virtual auto is_server_ready() -> std::tuple<call_result, bool> = 0;
    virtual auto server_metadata() -> std::tuple<call_result, server_metadata> = 0;
    
    virtual auto model_list() -> std::tuple<call_result, std::vector<std::string>> = 0;
    virtual auto is_model_ready(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, bool> = 0;
    virtual auto model_load(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, bool> = 0;
    virtual auto model_unload(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, bool> = 0;
    virtual auto model_metadata(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, model_metadata> = 0;

    virtual auto infer(const tie::infer_request& infer_request)  -> std::tuple<call_result, tie::infer_response> = 0;
};

}
