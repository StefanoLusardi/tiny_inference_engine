#include "rpc_model_list.hpp"

namespace tie::server
{
rpc_model_list::rpc_model_list(
    const uint64_t id,
    const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
    const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
    const std::shared_ptr<engine::engine_interface>& engine)
    : rpc_unary_async("rpc_model_list", id, service, cq, engine)
    , rpc_io<io_request_t, io_response_t>()
{
}

rpc_model_list::~rpc_model_list() { }

void rpc_model_list::setup_request() 
{
    _service->RequestModelList(&context, &request, &response_writer, _cq.get(), _cq.get(), static_cast<void*>(this));
}

void rpc_model_list::create_rpc() 
{
    rpc_pool::get().create_rpc<rpc_model_list>(_service, _cq, _engine)->execute();
}

void rpc_model_list::process_request()
{
    const auto model_list = _engine->model_list();
    for(auto&& model : model_list)
        response.add_models(model);
}

void rpc_model_list::write_response()
{
    response_writer.Finish(response, grpc::Status::OK, static_cast<void*>(this));
}

}

/*
message ModelListRequest {}
message ModelListResponse
{
    repeated string models = 1;
}
*/