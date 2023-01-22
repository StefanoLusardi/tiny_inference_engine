#include "grpc_client.hpp"
#include <grpcpp/create_channel.h>


namespace tie::client
{
grpc_client::grpc_client(const std::string& channel_address)
    : _stub{ inference::GRPCInferenceService::NewStub(grpc::CreateChannel(channel_address, grpc::InsecureChannelCredentials())) }
    , _tensor_converter{ std::make_unique<tie::client::tensor_converter>() }
{
    // _async_completion_queue = std::make_unique<grpc::CompletionQueue>();
    // for (auto thread_idx = 0; thread_idx < num_async_threads; ++thread_idx)
    //     _async_grpc_threads.emplace_back(std::thread([this](const std::shared_ptr<grpc::CompletionQueue>& cq) { rpc_handler(cq); }, _async_completion_queue));
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

auto grpc_client::server_metadata() -> std::tuple<call_result, tie::client::server_metadata>
{
    inference::ServerMetadataRequest request;
    inference::ServerMetadataResponse response;
    grpc::ClientContext context;

    grpc::Status rpc_status = _stub->ServerMetadata(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Server Metadata - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, {} };
    }

    const std::vector<std::string> server_extensions = { response.extensions().begin(), response.extensions().end() };

    tie::client::server_metadata metadata = {response.name(), response.version(), std::move(server_extensions) };

    SPDLOG_INFO("Server Metadata:");
    SPDLOG_INFO("  server_name: {}", metadata.server_name);
    SPDLOG_INFO("  server_version: {}", metadata.server_version);
    
    for (auto&& extension : metadata.server_extensions)
        SPDLOG_INFO("  server_extensions: {}", extension);

    return { call_result::OK, metadata };
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

auto grpc_client::model_metadata(const std::string& model_name, const std::string& model_version) -> std::tuple<call_result, tie::client::model_metadata>
{
    inference::ModelMetadataRequest request;
    inference::ModelMetadataResponse response;
    grpc::ClientContext context;

    request.set_name(model_name);
    request.set_version(model_version);

    grpc::Status rpc_status = _stub->ModelMetadata(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Model Metadata - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, {} };
    }

    tie::client::model_metadata metadata;

    metadata.model_name = response.name();
    metadata.model_versions = std::vector<std::string>{ response.versions().begin(), response.versions().end() };
    metadata.platform = response.platform();
    
    for(auto&& input : response.inputs())
    {
        std::vector<uint64_t> shape { input.shape().begin(), input.shape().end() };
        metadata.inputs.push_back({input.name(), input.datatype(), shape});
    }

    for(auto&& output : response.outputs())
    {
        std::vector<uint64_t> shape { output.shape().begin(), output.shape().end() };
        metadata.outputs.push_back({output.name(), output.datatype(), shape});
    }

    SPDLOG_INFO("Model Metadata:");
    SPDLOG_INFO("  name: {}", metadata.model_name);
    for(auto&& version : metadata.model_versions)
        SPDLOG_INFO("  version: {}", version);
    SPDLOG_INFO("  platform: {}", metadata.platform);

    // for(auto&& input : metadata.inputs)
    //     SPDLOG_INFO("  input: {}", input);

    // for(auto&& output : metadata.outputs)
    //     SPDLOG_INFO("  output: {}", output);
    
    return { call_result::OK, metadata };
}

auto grpc_client::infer(const infer_request& infer_request) -> std::tuple<call_result, infer_response>
{
    inference::ModelInferResponse response;
    grpc::ClientContext context;

    std::map<std::string, std::string> metadata {};
    for (auto&& [key, value] : metadata)
    {
        context.AddMetadata(key, value);
    }
    
    constexpr auto deadline_timeout = std::chrono::milliseconds(200);
    const auto deadline = std::chrono::system_clock::now() + deadline_timeout;
    // context.set_deadline(deadline);
    context.set_compression_algorithm(grpc_compression_algorithm::GRPC_COMPRESS_NONE);

    inference::ModelInferRequest request = _tensor_converter->get_infer_request(infer_request);
    grpc::Status rpc_status = _stub->ModelInfer(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Model Infer - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, {} };
    }

    tie::client::infer_response infer_response = _tensor_converter->get_infer_response(response);
    return { call_result::OK, infer_response };
}

void grpc_client::rpc_handler(const std::shared_ptr<grpc::CompletionQueue>& cq)
{
    // void* tag;
    // bool ok = false;
    
    // while (cq->Next(&tag, &ok))
    // {
    //     if (!ok)
    //     {
    //         SPDLOG_WARN("Client stream closed");
    //         break;
    //     }

    //     auto rpc = static_cast<AsyncClientCall*>(tag);
    //     rpc->proceed(ok);
    // }
}

}
