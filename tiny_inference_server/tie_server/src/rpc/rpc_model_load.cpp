#include "rpc_model_load.hpp"

namespace tie::server
{
rpc_model_load::rpc_model_load(
    const uint64_t id,
    const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
    const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
    const std::shared_ptr<engine::engine_interface>& engine)
    : rpc_unary_async("rpc_model_load", id, service, cq, engine)
    , rpc_io<io_request_t, io_response_t>()
{
}

rpc_model_load::~rpc_model_load() { }

void rpc_model_load::setup_request()
{
    _service->RequestModelLoad(&context, &request, &response_writer, _cq.get(), _cq.get(), static_cast<void*>(this));
}

void rpc_model_load::create_rpc()
{
    rpc_pool::get().create_rpc<rpc_model_load>(_service, _cq, _engine)->execute();
}

void rpc_model_load::process_request()
{
    std::cout << "rpc_model_load::process_request - begin" << std::endl;
    const bool is_model_loaded = _engine->model_load(request.name(), "");
    std::cout << "rpc_model_load::process_request - end" << std::endl;
}

void rpc_model_load::write_response()
{
    response_writer.Finish(response, grpc::Status::OK, static_cast<void*>(this));
}

}

/*
message ModelLoadRequest
{
    string name = 1;
    map<string, InferParameter> parameters = 2;
}
message ModelLoadResponse {}

message InferParameter
{
    oneof parameter_choice
    {
        bool bool_param = 1;
        int64 int64_param = 2;
        string string_param = 3;
        double double_param = 4;
    }
}
*/