#pragma once

#include "rpc_unary_async.hpp"
#include "rpc_io.hpp"

namespace tie::server
{
class rpc_model_load 
    : public rpc_unary_async
    , public rpc_io<inference::ModelLoadRequest, inference::ModelLoadResponse>
{
public:
    explicit rpc_model_load(
        const uint64_t id,
        const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine);
    ~rpc_model_load() override;

    void setup_request() override;
    void create_rpc() override;
    void process_request() override;
    void write_response() override;
};

}

/*
message ModelLoadRequest
{
    string name = 1;
    map<string, InferParameter> parameters = 2;
}
message ModelLoadResponse {}

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
*/