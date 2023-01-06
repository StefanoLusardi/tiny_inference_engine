#pragma once

#include <tie_client/client_interface.hpp>
#include <services.grpc.pb.h>

#if defined(TIE_CLIENT_ENABLE_LOG)
    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#else
    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_OFF
#endif
#include <spdlog/spdlog.h>

#include <vector>
#include <memory>
#include <string>
#include <thread>

namespace tie::client
{
class grpc_client final : public client_interface
{
public:
    explicit grpc_client(const std::string& channel_address);
    ~grpc_client() override;

    auto is_server_live() -> std::tuple<call_result, bool> override;
    auto is_server_ready() -> std::tuple<call_result, bool> override;
    auto server_metadata() -> std::tuple<call_result, tie::client::server_metadata> override;
    
    auto model_list() -> std::tuple<call_result, std::vector<std::string>> override;
    auto is_model_ready(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, bool> override;
    auto model_load(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, bool> override;
    auto model_unload(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, bool> override;
    auto model_metadata(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, tie::client::model_metadata> override;

    auto infer(const infer_request& infer_request) -> std::tuple<call_result, infer_response> override;

private:
    void rpc_handler(const std::shared_ptr<grpc::CompletionQueue>& cq);

    std::unique_ptr<inference::GRPCInferenceService::Stub> _stub;

    std::vector<std::thread> _async_grpc_threads;
    std::shared_ptr<grpc::CompletionQueue> _async_completion_queue;

    unsigned num_async_threads = 1u;
};

}
