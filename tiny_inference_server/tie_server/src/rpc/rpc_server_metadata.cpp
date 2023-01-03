#include "rpc_server_metadata.hpp"

namespace tie::server
{
rpc_server_metadata::rpc_server_metadata(
    const uint64_t id,
    const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
    const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
    const std::shared_ptr<engine::engine_interface>& engine)
    : rpc_unary_async("rpc_server_metadata", id, service, cq, engine)
    , rpc_io<io_request_t, io_response_t>()
{
}

rpc_server_metadata::~rpc_server_metadata() { }

void rpc_server_metadata::setup_request()
{
    _service->RequestServerMetadata(&context, &request, &response_writer, _cq.get(), _cq.get(), static_cast<void*>(this));
}

void rpc_server_metadata::create_rpc()
{
    rpc_pool::get().create_rpc<rpc_server_metadata>(_service, _cq, _engine)->execute();
}

void rpc_server_metadata::process_request()
{
    response.set_name("Tiny Inference Server");
    response.set_version("0.1.0");
}

void rpc_server_metadata::write_response()
{
    response_writer.Finish(response, grpc::Status::OK, static_cast<void*>(this));
}

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