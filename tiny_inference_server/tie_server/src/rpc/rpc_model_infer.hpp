#pragma once

#include "rpc_unary_async.hpp"
#include "rpc_io.hpp"
#include <thread>

namespace tie::server
{

class rpc_model_infer
    : public rpc_unary_async
    , public rpc_io<inference::ModelInferRequest, inference::ModelInferResponse>
{
public:
    explicit rpc_model_infer(
        const uint64_t id,
        const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine)
        : rpc_unary_async("rpc_model_infer", id, service, cq, engine)
        , rpc_io<io_request_t, io_response_t>()
    {
    }

    ~rpc_model_infer() override { }

    void setup_request() override
    {
        _service->RequestModelInfer(&context, &request, &response_writer, _cq.get(), _cq.get(), static_cast<void*>(this));
    }

    void create_rpc() override
    {
        rpc_pool::get().create_rpc<rpc_model_infer>(_service, _cq, _engine)->execute();
    }

    void process_request() override
    {
        infer();
    }

    void write_response() override
    {
        response_writer.Finish(response, grpc::Status::OK, static_cast<void*>(this));
    }

    void infer()
    {
        /*
        tie::engine::infer_request request;
        request.data = {request.data().cbegin(), request.data().cend()};
        request.model_name = request.model_name();
        request.shape = { request.shape().begin(), request.shape().end() };
        
        spdlog::trace("SingleInferenceCall - PROCESS - Request - Model: {} - Data size: {}", request.model_name, request.data.size());

        tie::engine::infer_response response = _engine->infer(request);

        for (auto [tensor_name, tensor_info] : response.tensors)
        {
            tie::InferResponse::TensorInfo response_tensor_info;

            auto data = response_tensor_info.mutable_data();
            *data = std::string(static_cast<char*>(tensor_info.data), static_cast<char*>(tensor_info.data) + tensor_info.count);
            response_tensor_info.set_count(tensor_info.count);
            response_tensor_info.mutable_shape()->Assign(tensor_info.shape.begin(), tensor_info.shape.end() );                    
            response_tensor_info.set_type(tie::InferResponse::TensorInfo::data_type(tensor_info.type));

            (*response.mutable_tensors())[tensor_name] = response_tensor_info;
        }
        */

        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

};

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