#include "rpc_model_infer.hpp"
#include <thread>

namespace tie::server
{
rpc_model_infer::rpc_model_infer(
    const uint64_t id,
    const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
    const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
    const std::shared_ptr<engine::engine_interface>& engine)
    : rpc_unary_async("rpc_model_infer", id, service, cq, engine)
    , rpc_io<io_request_t, io_response_t>()
{
}

rpc_model_infer::~rpc_model_infer() { }

void rpc_model_infer::setup_request()
{
    _service->RequestModelInfer(&context, &request, &response_writer, _cq.get(), _cq.get(), static_cast<void*>(this));
}

void rpc_model_infer::create_rpc()
{
    rpc_pool::get().create_rpc<rpc_model_infer>(_service, _cq, _engine)->execute();
}

void rpc_model_infer::process_request()
{
    spdlog::trace("model_name: {}", request.model_name());
    spdlog::trace("model_version: {}", request.model_version());
    spdlog::trace("id: {}", request.id());
    
    for (auto&& input : request.inputs())
    {
        spdlog::trace("id: {}", input.name());
        spdlog::trace("id: {}", input.datatype());
        // spdlog::trace("id: {}", input.());
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));
}

void rpc_model_infer::write_response()
{
    response_writer.Finish(response, grpc::Status::OK, static_cast<void*>(this));
}

}

/*
message InferParameter
{
    oneof parameter_choice
    {
        bool bool_param = 1;
        int64 int64_param = 2;
        string string_param = 3;
        double double_param = 4;
    }
}
message InferTensorContents
{
    repeated bool bool_contents = 1;
    repeated int32 int_contents = 2;
    repeated int64 int64_contents = 3;
    repeated uint32 uint_contents = 4;
    repeated uint64 uint64_contents = 5;
    repeated float fp32_contents = 6;
    repeated double fp64_contents = 7;
    repeated bytes bytes_contents = 8;
}
message ModelInferRequest
{
    message InferInputTensor
    {
        string name = 1;
        string datatype = 2;
        repeated int64 shape = 3;
        map<string, InferParameter> parameters = 4;
        InferTensorContents contents = 5;
    }
    message InferRequestedOutputTensor
    {
        string name = 1;
        map<string, InferParameter> parameters = 2;
    }
    string model_name = 1;
    string model_version = 2;
    string id = 3;
    map<string, InferParameter> parameters = 4;
    repeated InferInputTensor inputs = 5;
    repeated InferRequestedOutputTensor outputs = 6;
    repeated bytes raw_input_contents = 7;
}
message ModelInferResponse
{
    message InferOutputTensor
    {
        string name = 1;
        string datatype = 2;
        repeated int64 shape = 3;
        map<string, InferParameter> parameters = 4;
        InferTensorContents contents = 5;
    }
    string model_name = 1;
    string model_version = 2;
    string id = 3;
    map<string, InferParameter> parameters = 4;
    repeated InferOutputTensor outputs = 5;
    repeated bytes raw_output_contents = 6;
}
*/