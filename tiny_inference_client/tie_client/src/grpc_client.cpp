#include "grpc_client.hpp"
#include <tie_client/call_result.hpp>
#include <tie_client/infer_response.hpp>
#include <grpcpp/create_channel.h>

namespace tie::client
{

struct AsyncClientCall
{
    grpc::ClientContext context;
    grpc::Status result_code;
    virtual bool proceed(bool ok) = 0;
};

template<class ResponseT>
struct AsyncClientCallback : public AsyncClientCall
{
    ResponseT response;
    void set_response_callback(std::function<void(ResponseT)> response_callback) { _on_response_callback = response_callback; }

protected:
    std::function<void(ResponseT)> _on_response_callback;
};

template<class ResponseT>
struct AsyncClientUnaryCall : public AsyncClientCallback<ResponseT>
{
    std::unique_ptr<grpc::ClientAsyncResponseReader<ResponseT>> rpc;

    bool proceed(bool ok) override
    {
        if (ok)
        {
            if (this->result_code.ok())
            {
                // SPDLOG_INFO(this->response.message());
                if (this->_on_response_callback) this->_on_response_callback(this->response);
            }
            else
            {
                SPDLOG_INFO("RPC failed: (" << this->result_code.error_code() << ") " << this->result_code.error_message());
            }
        }

        return false;
    }
};

grpc_client::grpc_client(const std::string& channel_address)
    : _stub{ inference::GRPCInferenceService::NewStub(grpc::CreateChannel(channel_address, grpc::InsecureChannelCredentials())) }
{
    // _async_completion_queue = std::make_unique<grpc::CompletionQueue>();
    // for (auto thread_idx = 0; thread_idx < num_async_threads; ++thread_idx)
    //     _async_grpc_threads.emplace_back(std::thread([this](const std::shared_ptr<grpc::CompletionQueue>& cq) { grpc_thread_worker(cq); }, _async_completion_queue));
}

grpc_client::~grpc_client()
{
    SPDLOG_INFO("Shutting down client");

    // _async_completion_queue->Shutdown();
    // for (auto&& t : _async_grpc_threads)
    //     if (t.joinable()) t.join();
}

auto grpc_client::is_server_live() -> std::tuple<call_result, bool>
{
    inference::ServerLiveRequest request;
    inference::ServerLiveResponse response;
    grpc::ClientContext context;

    const grpc::Status rpc_status = _stub->ServerLive(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Server Live - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, false };
    }

    const bool is_server_live = response.live();
    SPDLOG_INFO("Server Live: {}", is_server_live);

    return { call_result::OK, is_server_live };
}

auto grpc_client::is_server_ready() -> std::tuple<call_result, bool>
{
    inference::ServerReadyRequest request;
    inference::ServerReadyResponse response;
    grpc::ClientContext context;

    const grpc::Status rpc_status = _stub->ServerReady(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Server Ready - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, false };
    }

    const bool is_server_ready = response.ready();
    SPDLOG_INFO("Server Ready: {}", is_server_ready);

    return { call_result::OK, is_server_ready };
}

auto grpc_client::server_metadata() -> std::tuple<call_result, bool>
{
    inference::ServerMetadataRequest request;
    inference::ServerMetadataResponse response;
    grpc::ClientContext context;

    // for (auto&& [key, value] : metadata)
    // {
    //     context.AddMetadata(key, value);
    // }

    grpc::Status rpc_status = _stub->ServerMetadata(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Server Metadata - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, false };
    }

    const auto server_name = response.name();
    const auto server_version = response.version();
    const auto server_extensions = response.extensions();

    SPDLOG_INFO("Server Metadata:");
    SPDLOG_INFO("  server_name: {}", server_name);
    SPDLOG_INFO("  server_version: {}", server_version);
    // SPDLOG_INFO("  server_extensions: {}", server_extensions);

    return { call_result::OK, true };
}

auto grpc_client::is_model_ready(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, bool>
{
    inference::ModelReadyRequest request;
    inference::ModelReadyResponse response;
    grpc::ClientContext context;

    request.set_name(model_name);
    request.set_version(model_version);

    grpc::Status rpc_status = _stub->ModelReady(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Model Ready - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, false };
    }

    const auto is_model_ready = response.ready();

    SPDLOG_INFO("Model Ready:");
    SPDLOG_INFO("  name: {}", model_name);
    SPDLOG_INFO("  version: {}", model_version);

    return { call_result::OK, is_model_ready };
}

auto grpc_client::model_list() -> std::tuple<call_result, std::vector<std::string>>
{
    inference::ModelListRequest request;
    inference::ModelListResponse response;
    grpc::ClientContext context;

    grpc::Status rpc_status = _stub->ModelList(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Model List - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, {} };
    }
    
    const auto models = response.models();
    const std::vector<std::string> model_list(models.begin(), models.end());

    // SPDLOG_INFO("Model List: {}", model_list);
    
    return { call_result::OK, model_list };
}

auto grpc_client::model_load(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, bool>
{
    inference::ModelLoadRequest request;
    inference::ModelLoadResponse response;
    grpc::ClientContext context;

    request.set_name(model_name);
    grpc::Status rpc_status = _stub->ModelLoad(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Model Load - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, false };
    }

    SPDLOG_INFO("Model Load:");
    SPDLOG_INFO("  name: {}", model_name);
    SPDLOG_INFO("  version: {}", model_version);
        
    return { call_result::OK, true };
}

auto grpc_client::model_unload(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, bool>
{
    inference::ModelUnloadRequest request;
    inference::ModelUnloadResponse response;
    grpc::ClientContext context;

    request.set_name(model_name);
    grpc::Status rpc_status = _stub->ModelUnload(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Model Unload - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, false };
    }

    SPDLOG_INFO("Model Unoad:");
    SPDLOG_INFO("  name: {}", model_name);
    SPDLOG_INFO("  version: {}", model_version);
        
    return { call_result::OK, true };
}

auto grpc_client::model_metadata(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, bool>
{
    inference::ModelMetadataRequest request;
    inference::ModelMetadataResponse response;
    grpc::ClientContext context;

    request.set_name(model_name);
    grpc::Status rpc_status = _stub->ModelMetadata(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Model Metadata - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, false };
    }

    SPDLOG_INFO("Model Metadata:");
    SPDLOG_INFO("  name: {}", model_name);
    SPDLOG_INFO("  version: {}", model_version);
    
    return { call_result::OK, true };
}


auto grpc_client::infer(const tie::infer_request& infer_request) -> std::tuple<call_result, tie::infer_response>
{
    inference::ModelInferRequest request;
    inference::ModelInferResponse response;
    grpc::ClientContext context;

    grpc::Status rpc_status = _stub->ModelInfer(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Model Infer - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, {} };
    }
        
    return { call_result::OK, {} };
}

/*
    SPDLOG_INFO("infer");

    tie::InferRequest request;
    request.set_data(infer_request.data.data(), infer_request.data.size());
    request.set_model_name(infer_request.model_name.data(), infer_request.model_name.size());

    tie::InferResponse response;
    grpc::ClientContext context;

    grpc::Status status = _stub->Infer(&context, request, &response);
    if (status.ok())
        SPDLOG_INFO(response.error_message());
    else
        SPDLOG_INFO("RPC failed: (" << status.error_code() << ") " << status.error_message());

    tie::infer_response infer_response;

    auto n = "squeezenet0_flatten0_reshape0";
    for (auto&& [tensor_name, tensor_info] : response.tensors())
    {
        infer_response.tensors.emplace(n, tie::infer_response::tensor_info{});

        infer_response.tensors[n].data = (void*)tensor_info.data().data();
        infer_response.tensors[n].count = tensor_info.count();
        infer_response.tensors[n].shape = { tensor_info.shape().begin(), tensor_info.shape().end() };
        infer_response.tensors[n].type = static_cast<tie::data_type>(tensor_info.type());
    }

    return infer_response;
*/

void grpc_client::rpc_handler(const std::shared_ptr<grpc::CompletionQueue>& cq)
{
    void* tag;
    bool ok = false;
    
    while (cq->Next(&tag, &ok))
    {
        if (!ok)
        {
            SPDLOG_WARN("Client stream closed");
            break;
        }

        auto rpc = static_cast<AsyncClientCall*>(tag);
        rpc->proceed(ok);
    }
}

}
