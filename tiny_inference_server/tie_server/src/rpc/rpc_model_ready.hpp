#pragma once

#include "rpc_unary_async.hpp"
#include "rpc_io.hpp"

namespace tie::server
{
class rpc_model_ready 
    : public rpc_unary_async
    , public rpc_io<inference::ModelReadyRequest, inference::ModelReadyResponse>
{
public:
    explicit rpc_model_ready(
        const uint64_t id,
        const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine);
    ~rpc_model_ready() override;

    void setup_request() override;
    void create_rpc() override;
    void process_request() override;
    void write_response() override;
};

}

/*
message ModelReadyRequest
{
    string name = 1;
    string version = 2;
}
message ModelReadyResponse
{
    bool ready = 1;
}
*/