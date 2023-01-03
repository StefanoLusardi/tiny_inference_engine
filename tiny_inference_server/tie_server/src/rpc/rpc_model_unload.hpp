#pragma once

#include "rpc_unary_async.hpp"
#include "rpc_io.hpp"

namespace tie::server
{
class rpc_model_unload 
    : public rpc_unary_async
    , public rpc_io<inference::ModelUnloadRequest, inference::ModelUnloadResponse>
{
public:
    explicit rpc_model_unload(
        const uint64_t id,
        const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine);

    ~rpc_model_unload() override;

    void setup_request() override;
    void create_rpc() override;
    void process_request() override;
    void write_response() override;
};

}

/*
message ModelUnloadRequest
{
    string name = 1;
}
message ModelUnloadResponse {}
*/