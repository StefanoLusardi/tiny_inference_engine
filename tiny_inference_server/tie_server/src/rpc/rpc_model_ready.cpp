#include "rpc_model_ready.hpp"

namespace tie::server
{
rpc_model_ready::rpc_model_ready(
    const uint64_t id,
    const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
    const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
    const std::shared_ptr<engine::engine_interface>& engine)
    : rpc_unary_async("rpc_model_ready", id, service, cq, engine)
    , rpc_io<io_request_t, io_response_t>()
{
}

rpc_model_ready::~rpc_model_ready() { }

void rpc_model_ready::setup_request()
{
    _service->RequestModelReady(&context, &request, &response_writer, _cq.get(), _cq.get(), static_cast<void*>(this));
}

void rpc_model_ready::create_rpc()
{
    rpc_pool::get().create_rpc<rpc_model_ready>(_service, _cq, _engine)->execute();
}

void rpc_model_ready::process_request()
{
    const bool is_model_ready = _engine->is_model_ready(request.name(), request.version());
    response.set_ready(is_model_ready);
}

void rpc_model_ready::write_response()
{
    response_writer.Finish(response, grpc::Status::OK, static_cast<void*>(this));
}

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