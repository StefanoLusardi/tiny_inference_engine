#include "grpc_client.hpp"
#include "tie_client/infer_response.hpp"
// #include "tie_client/server_metadata.hpp"
// #include "tie_client/model_metadata.hpp"
// #include <tie_client/call_result.hpp>
// #include <tie_client/infer_response.hpp>
// #include <tie_client/server_metadata.hpp>
#include <grpcpp/create_channel.h>
#include <numeric>

namespace tie::client
{
grpc_client::grpc_client(const std::string& channel_address)
    : _stub{ inference::GRPCInferenceService::NewStub(grpc::CreateChannel(channel_address, grpc::InsecureChannelCredentials())) }
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

namespace util
{
    template <typename T, typename... Ts>
    struct is_any : std::disjunction<std::is_same<T, Ts>...> {};

    template <typename T, typename... Ts>
    inline constexpr bool is_any_v = is_any<T, Ts...>::value;
}

template<typename T, typename Tensor>
constexpr auto* getTensorContents(Tensor* tensor)
{
    if constexpr (std::is_same_v<T, bool>)
    {
        if constexpr (std::is_const_v<Tensor>)
        {
            return tensor->contents().bool_contents().data();
        }
        else
        {
            return tensor->mutable_contents()->mutable_bool_contents();
        }
    }
    
    if constexpr (util::is_any_v<T, uint8_t, uint16_t, uint32_t>)
    {
        if constexpr (std::is_const_v<Tensor>)
        {
            return tensor->contents().uint_contents().data();
        }
        else
        {
            return tensor->mutable_contents()->mutable_uint_contents();
        }
    }
    
    if constexpr (std::is_same_v<T, uint64_t>)
    {
        if constexpr (std::is_const_v<Tensor>)
        {
            return tensor->contents().uint64_contents().data();
        }
        else
        {
            return tensor->mutable_contents()->mutable_uint64_contents();
        }
    }
    
    if constexpr (util::is_any_v<T, int8_t, int16_t, int32_t>)
    {
        if constexpr (std::is_const_v<Tensor>)
        {
            return tensor->contents().int_contents().data();
        }
        else
        {
            return tensor->mutable_contents()->mutable_int_contents();
        }
    }

    if constexpr (std::is_same_v<T, int64_t>)
    {
        if constexpr (std::is_const_v<Tensor>)
        {
            return tensor->contents().int64_contents().data();
        }
        else
        {
            return tensor->mutable_contents()->mutable_int64_contents();
        }
    }
    
    if constexpr (util::is_any_v<T, float>) // fp16, 
    {
        if constexpr (std::is_const_v<Tensor>)
        {
            return tensor->contents().fp32_contents().data();
        }
        else
        {
            return tensor->mutable_contents()->mutable_fp32_contents();
        }
    }
    
    if constexpr (std::is_same_v<T, double>)
    {
        if constexpr (std::is_const_v<Tensor>)
        {
            return tensor->contents().fp64_contents().data();
        }
        else
        {
            return tensor->mutable_contents()->mutable_fp64_contents();
        }
    }
    
    if constexpr (std::is_same_v<T, char>)
    {
        if constexpr (std::is_const_v<Tensor>)
        {
            return tensor->contents().bytes_contents().data();
        }
        else
        {
            return tensor->mutable_contents()->mutable_bytes_contents();
        }
    }

    // static_assert(!sizeof(T), "Invalid type to AddDataToTensor");
}

template<typename T, typename Tensor>
struct SetInputData
{
    constexpr void operator()(Tensor* tensor, const void* source_data, size_t size)
    {
        const auto* data = static_cast<const T*>(source_data);
        auto* contents = getTensorContents<T>(tensor);

        if constexpr (std::is_same_v<T, char>)
        {
            contents->Add(data);
        }
        // else if constexpr (std::is_same_v<T, fp16>)
        // {
        //     for (auto i = 0U; i < size; ++i)
        //     {
        //         contents->Add(static_cast<float>(data[i]));
        //     }
        // }
        else
        {
            for (auto i = 0U; i < size; ++i)
            {
                contents->Add(data[i]);
            }
        }
    }
};

template<typename T, typename Tensor>
struct SetOutputData
{
    /*constexpr*/ void operator()(Tensor* tensor, InferenceResponseOutput* output, size_t size)
    {
        std::vector<std::byte> data;
        const auto bytes_to_copy = size * sizeof(T);
        data.resize(bytes_to_copy);
        const auto* contents = getTensorContents<T>(tensor);
        if constexpr (std::is_same_v<T, char>)
        {
            std::memcpy(data.data(), contents, size * sizeof(std::byte));
            output->data = data.data();
            // output->data = std::move(data);
        }
        else
        {
            if constexpr (util::is_any_v<T, int8_t, uint8_t, int16_t, uint16_t>) // fp16
            {
                for (auto i = 0U; i < size; ++i)
                {
                    std::memcpy(&(data[i * sizeof(T)]), &(contents[i]), sizeof(T));
                }
            }
            else
            {
                std::memcpy(data.data(), contents, bytes_to_copy);
            }
            output->data = data.data();
            // output->data = std::move(data);
        }
    }
};

/*
template<typename F, typename... Args>
auto switchOverTypes(F f, tie::client::data_type type, [[maybe_unused]] const Args&... args)
{
    switch (type)
    {
        case data_type::Bool:
        {
            return f.template operator()<bool>(args...);
        }
        case data_type::Uint8:
        {
            return f.template operator()<uint8_t>(args...);
        }
        case data_type::Uint16:
        {
            return f.template operator()<uint16_t>(args...);
        }
        case data_type::Uint32:
        {
            return f.template operator()<uint32_t>(args...);
        }
        case data_type::Uint64:
        {
            return f.template operator()<uint64_t>(args...);
        }
        case data_type::Int8:
        {
            return f.template operator()<int8_t>(args...);
        }
        case data_type::Int16:
        {
            return f.template operator()<int16_t>(args...);
        }
        case data_type::Int32:
        {
            return f.template operator()<int32_t>(args...);
        }
        case data_type::Int64:
        {
            return f.template operator()<int64_t>(args...);
        }
        // case data_type::Fp16: {
        //   return f.template operator()<fp16>(args...);
        // }
        case data_type::Fp32:
        {
            return f.template operator()<float>(args...);
        }
        case data_type::Fp64:
        {
            return f.template operator()<double>(args...);
        }
        case data_type::String:
        {
            return f.template operator()<char>(args...);
        }
        default: throw;
    }
}
*/

template<typename F, typename TensorT, typename... Args>
auto switchOverTypes(F f, tie::client::data_type type, TensorT tensor, [[maybe_unused]] const Args&... args)
{
    switch (type)
    {
        case data_type::Bool:
        {
            // return std::invoke(f<bool, decltype(tensor)>, args...);
        }
        case data_type::Uint8:
        {
            return f.template operator()<uint8_t>(args...);
        }
        case data_type::Uint16:
        {
            return f.template operator()<uint16_t>(args...);
        }
        case data_type::Uint32:
        {
            return f.template operator()<uint32_t>(args...);
        }
        case data_type::Uint64:
        {
            return f.template operator()<uint64_t>(args...);
        }
        case data_type::Int8:
        {
            return f.template operator()<int8_t>(args...);
        }
        case data_type::Int16:
        {
            return f.template operator()<int16_t>(args...);
        }
        case data_type::Int32:
        {
            return f.template operator()<int32_t>(args...);
        }
        case data_type::Int64:
        {
            return f.template operator()<int64_t>(args...);
        }
        // case data_type::Fp16: {
        //   return f.template operator()<fp16>(args...);
        // }
        case data_type::Fp32:
        {
            return f.template operator()<float>(args...);
        }
        case data_type::Fp64:
        {
            return f.template operator()<double>(args...);
        }
        case data_type::String:
        {
            return f.template operator()<char>(args...);
        }
        default: throw;
    }
}

template<template<typename, typename> class func_t, typename TensorT, typename... Args>
void callable_wrapper(tie::client::data_type type, TensorT* tensor, Args... args)
{
    switch(type)
    {
        case data_type::Bool:   return std::invoke(func_t<bool, TensorT>(), tensor, args...);
        case data_type::Uint8:  return std::invoke(func_t<uint8_t, TensorT>(), tensor, args...);
        case data_type::Uint16: return std::invoke(func_t<uint16_t, TensorT>(), tensor, args...);
        case data_type::Uint32: return std::invoke(func_t<uint32_t, TensorT>(), tensor, args...);
        case data_type::Uint64: return std::invoke(func_t<uint64_t, TensorT>(), tensor, args...);
        case data_type::Int8:   return std::invoke(func_t<int8_t, TensorT>(), tensor, args...);
        case data_type::Int16:  return std::invoke(func_t<int16_t, TensorT>(), tensor, args...);
        case data_type::Int32:  return std::invoke(func_t<int32_t, TensorT>(), tensor, args...);
        case data_type::Int64:  return std::invoke(func_t<int64_t, TensorT>(), tensor, args...);
        case data_type::Fp16:   return; // std::invoke(func_t<uint64_t, TensorT>(), tensor, args...);
        case data_type::Fp32:   return std::invoke(func_t<float, TensorT>(), tensor, args...);
        case data_type::Fp64:   return std::invoke(func_t<double, TensorT>(), tensor, args...);
        case data_type::String: return std::invoke(func_t<char, TensorT>(), tensor, args...);
        case data_type::Unknown: static_assert("Unknown DataType"); return;
    }
}

auto grpc_client::infer(const infer_request& infer_request) -> std::tuple<call_result, infer_response>
{
    inference::ModelInferRequest request;
    inference::ModelInferResponse response;
    grpc::ClientContext context;

    std::map<std::string, std::string> metadata {};
    for (auto&& [key, value] : metadata)
    {
        context.AddMetadata(key, value);
    }
    
    const auto deadline_timeout = std::chrono::milliseconds(200);
    const auto deadline = std::chrono::system_clock::now() + deadline_timeout;
    // context.set_deadline(deadline);
    context.set_compression_algorithm(grpc_compression_algorithm::GRPC_COMPRESS_NONE);

    request.set_model_name(infer_request.model_name);
    request.set_model_version(infer_request.model_version);
    request.set_id(infer_request.id);

    for (auto&& input : infer_request.inputs)
    {
        auto tensor = request.add_inputs();
        tensor->set_name(input.name);
        tensor->set_datatype(input.datatype.str());

        for (auto&& shape : input.shape)
        {
            tensor->add_shape(shape);
        }

        // mapParametersToProto(input.parameters->data, tensor->mutable_parameters());
        const size_t input_size = std::accumulate(input.shape.begin(), input.shape.end(), 1, std::multiplies<>());
        callable_wrapper<SetInputData>(input.datatype, tensor, input.data, input_size);
    }

    grpc::Status rpc_status = _stub->ModelInfer(&context, request, &response);

    if (!rpc_status.ok())
    {
        SPDLOG_INFO("ERROR - Model Infer - code: {} - msg: {}", rpc_status.error_code(), rpc_status.error_message());
        return { call_result::ERROR, {} };
    }

    tie::client::infer_response infer_response;
    infer_response.model_name = response.model_name();
    infer_response.model_version = response.model_version();
    infer_response.id = response.id();

    for (const auto& tensor : response.outputs())
    {
        InferenceResponseOutput response_output;

        response_output.name = tensor.name();
        response_output.datatype = tie::client::data_type(tensor.datatype().c_str());
        
        std::vector<uint64_t> tensor_shape;
        tensor_shape.reserve(tensor.shape_size());
        
        auto output_size = 1U;
        for (const auto& shape : tensor.shape())
        {
            tensor_shape.push_back(static_cast<size_t>(shape));
            output_size *= shape;
        }

        response_output.shape = tensor_shape;
        callable_wrapper<SetOutputData>(response_output.datatype, &tensor, &response_output,  output_size);
        infer_response.addOutput(response_output);
    }

    return { call_result::OK, infer_response };
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
