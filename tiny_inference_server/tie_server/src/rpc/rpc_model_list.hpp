#pragma once

#include "rpc_unary_async.hpp"
#include "rpc_io.hpp"

namespace tie::server
{
class rpc_model_list 
    : public rpc_unary_async
    , public rpc_io<inference::ModelListRequest, inference::ModelListResponse>
{
public:
    explicit rpc_model_list(
        const uint64_t id,
        const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine);
    ~rpc_model_list() override;
    
    void setup_request() override;
    void create_rpc() override;
    void process_request() override;
    void write_response() override;
};

}

/*
message ModelListRequest {}
message ModelListResponse
{
    repeated string models = 1;
}
*/