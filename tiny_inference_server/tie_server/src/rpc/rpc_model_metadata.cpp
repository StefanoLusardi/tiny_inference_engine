#include "rpc_model_metadata.hpp"

namespace tie::server
{
rpc_model_metadata::rpc_model_metadata(
    const uint64_t id,
    const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
    const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
    const std::shared_ptr<engine::engine_interface>& engine)
    : rpc_unary_async("rpc_model_metadata", id, service, cq, engine)
    , rpc_io<io_request_t, io_response_t>()
{
}

rpc_model_metadata::~rpc_model_metadata() { }

void rpc_model_metadata::setup_request()
{
    _service->RequestModelMetadata(&context, &request, &response_writer, _cq.get(), _cq.get(), static_cast<void*>(this));
}

void rpc_model_metadata::create_rpc()
{
    rpc_pool::get().create_rpc<rpc_model_metadata>(_service, _cq, _engine)->execute();
}

void rpc_model_metadata::process_request()
{
    const auto model_metadata = _engine->model_metadata(request.name(), request.version());
}

void rpc_model_metadata::write_response()
{
    response_writer.Finish(response, grpc::Status::OK, static_cast<void*>(this));
}

}

/*
message ModelMetadataRequest
{
    string name = 1;
    string version = 2;
}

message ModelMetadataResponse
{
    message TensorMetadata
    {
        string name = 1;
        string datatype = 2;
        repeated int64 shape = 3;
    }
    string name = 1;
    repeated string versions = 2;
    string platform = 3;
    repeated TensorMetadata inputs = 4;
    repeated TensorMetadata outputs = 5;
}
*/