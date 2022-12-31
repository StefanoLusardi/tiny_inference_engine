#pragma once

#include "rpc_unary_async.hpp"
#include "rpc_io.hpp"

namespace tie::server
{
class rpc_server_metadata
    : public rpc_unary_async
    , public rpc_io<inference::ServerMetadataRequest, inference::ServerMetadataResponse>
{
public:
    explicit rpc_server_metadata(
        const uint64_t id,
        const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine)
        : rpc_unary_async("rpc_server_metadata", id, service, cq, engine)
        , rpc_io<io_request_t, io_response_t>()
    {
    }

    ~rpc_server_metadata() override { }

    void setup_request() override
    {
        _service->RequestServerMetadata(&context, &request, &response_writer, _cq.get(), _cq.get(), static_cast<void*>(this));
    }

    void create_rpc() override
    {
        rpc_pool::get().create_rpc<rpc_server_metadata>(_service, _cq, _engine)->execute();
    }

    void process_request() override
    {
        
    }

    void write_response() override
    {
        response_writer.Finish(response, grpc::Status::OK, static_cast<void*>(this));
    }
};

}

/*
message ServerMetadataRequest {}
message ServerMetadataResponse
{
    string name = 1;
    string version = 2;
    repeated string extensions = 3;
}
*/