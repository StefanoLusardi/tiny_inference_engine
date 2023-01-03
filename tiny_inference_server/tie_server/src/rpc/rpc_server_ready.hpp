#pragma once

#include "rpc_unary_async.hpp"
#include "rpc_io.hpp"

namespace tie::server
{
class rpc_server_ready
    : public rpc_unary_async
    , public rpc_io<inference::ServerReadyRequest, inference::ServerReadyResponse>
{
public:
    explicit rpc_server_ready(
        const uint64_t id,
        const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine);
    ~rpc_server_ready() override;
    
    void setup_request() override;
    void create_rpc() override;
    void process_request() override;
    void write_response() override;
};

}

/*
message ServerReadyRequest {}
message ServerReadyResponse
{
    bool ready = 1;
}
*/