#pragma once

#include "rpc_unary_async.hpp"
#include "rpc_io.hpp"

namespace tie::server
{
class rpc_server_live 
    : public rpc_unary_async
    , public rpc_io<inference::ServerLiveRequest, inference::ServerLiveResponse>
{
public:
    explicit rpc_server_live(
        const uint64_t id,
        const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine);
    ~rpc_server_live() override;

    void setup_request() override;
    void create_rpc() override;
    void process_request() override;
    void write_response() override;
};

}

/*
message ServerLiveRequest {}
message ServerLiveResponse
{
    bool live = 1;
}
*/