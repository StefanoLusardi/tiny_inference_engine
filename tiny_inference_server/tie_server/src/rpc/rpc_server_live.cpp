#include "rpc_server_live.hpp"

namespace tie::server
{
rpc_server_live::rpc_server_live(
    const uint64_t id,
    const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
    const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
    const std::shared_ptr<engine::engine_interface>& engine)
    : rpc_unary_async("rpc_server_live", id, service, cq, engine)
    , rpc_io<io_request_t, io_response_t>()
{
}

rpc_server_live::~rpc_server_live() { }

void rpc_server_live::setup_request()
{
    _service->RequestServerLive(&context, &request, &response_writer, _cq.get(), _cq.get(), static_cast<void*>(this));
}

void rpc_server_live::create_rpc()
{
    rpc_pool::get().create_rpc<rpc_server_live>(_service, _cq, _engine)->execute();
}

void rpc_server_live::process_request()
{
    const bool is_server_live = true;
    response.set_live(is_server_live);
}

void rpc_server_live::write_response()
{
    response_writer.Finish(response, grpc::Status::OK, static_cast<void*>(this));
}

}

/*
message ServerLiveRequest {}
message ServerLiveResponse
{
    bool live = 1;
}
*/