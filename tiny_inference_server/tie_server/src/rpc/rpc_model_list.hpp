#pragma once

#include "rpc_unary_async.hpp"
#include "rpc_io.hpp"

namespace tie::server
{
class rpc_model_list 
    : public rpc_unary_async
    , public rpc_io<inference::ModelMetadataRequest, inference::ModelMetadataResponse>
{
public:
    explicit rpc_model_list(
        const uint64_t id,
        const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine)
        : rpc_unary_async("rpc_model_list", id, service, cq, engine)
        , rpc_io<io_request_t, io_response_t>()
    {
    }

    ~rpc_model_list() override { }

    void setup_request() override
    {
        _service->RequestModelMetadata(&context, &request, &response_writer, _cq.get(), _cq.get(), static_cast<void*>(this));
    }

    void create_rpc() override
    {
        rpc_pool::get().create_rpc<rpc_model_list>(_service, _cq, _engine)->execute();
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
message ModelListRequest {}
message ModelListResponse
{
    repeated string models = 1;
}
*/