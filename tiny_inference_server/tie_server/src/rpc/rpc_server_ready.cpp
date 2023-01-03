#include "rpc_server_ready.hpp"

namespace tie::server
{
rpc_server_ready::rpc_server_ready(
    const uint64_t id,
    const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
    const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
    const std::shared_ptr<engine::engine_interface>& engine)
    : rpc_unary_async("rpc_server_ready", id, service, cq, engine)
    , rpc_io<io_request_t, io_response_t>()
{
}

rpc_server_ready::~rpc_server_ready() { }

void rpc_server_ready::setup_request()
{
    _service->RequestServerReady(&context, &request, &response_writer, _cq.get(), _cq.get(), static_cast<void*>(this));
}

void rpc_server_ready::create_rpc()
{
    rpc_pool::get().create_rpc<rpc_server_ready>(_service, _cq, _engine)->execute();
}

void rpc_server_ready::process_request()
{
    const bool is_server_ready = _engine->is_engine_ready();
    response.set_ready(is_server_ready);
}

void rpc_server_ready::write_response()
{
    response_writer.Finish(response, grpc::Status::OK, static_cast<void*>(this));
}

}

/*
message ServerReadyRequest {}
message ServerReadyResponse
{
    bool ready = 1;
}
*/