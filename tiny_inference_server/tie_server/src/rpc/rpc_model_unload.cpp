#include "rpc_model_unload.hpp"

namespace tie::server
{
rpc_model_unload::rpc_model_unload(
    const uint64_t id,
    const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
    const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
    const std::shared_ptr<engine::engine_interface>& engine)
    : rpc_unary_async("rpc_model_unload", id, service, cq, engine)
    , rpc_io<io_request_t, io_response_t>()
{
}

rpc_model_unload::~rpc_model_unload() { }

void rpc_model_unload::setup_request()
{
    _service->RequestModelUnload(&context, &request, &response_writer, _cq.get(), _cq.get(), static_cast<void*>(this));
}

void rpc_model_unload::create_rpc()
{
    rpc_pool::get().create_rpc<rpc_model_unload>(_service, _cq, _engine)->execute();
}

void rpc_model_unload::process_request()
{
    const bool is_model_unloaded = _engine->model_unload(request.name(), "");
}

void rpc_model_unload::write_response()
{
    response_writer.Finish(response, grpc::Status::OK, static_cast<void*>(this));
}

}

/*
message ModelUnloadRequest
{
    string name = 1;
}
message ModelUnloadResponse {}
*/