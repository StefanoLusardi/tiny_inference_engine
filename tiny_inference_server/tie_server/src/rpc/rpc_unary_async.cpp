#include "rpc_unary_async.hpp"

namespace tie::server
{
rpc_unary_async::rpc_unary_async(
    const char* name,
    const uint64_t id,
    const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
    const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
    const std::shared_ptr<tie::engine::engine_interface>& engine)
: rpc(name, id)
, _rpc_state{ rpc_status::CREATE }
, _service{ service }
, _cq{ cq }
, _engine{ engine }
{
    spdlog::trace("create rpc {} - {}", id, name);
}

rpc_unary_async::~rpc_unary_async()
{
    spdlog::trace("delete rpc {} - {}", id, name);
}

bool rpc_unary_async::execute()
{
    static auto hasher = std::hash<std::thread::id>();

    switch (_rpc_state)
    {
        case rpc_status::CREATE:
            spdlog::trace("[{}] rpc {} - {} (thread_id: {})", " CREATE ", id, name, hasher(std::this_thread::get_id()));
            setup_request();
            _rpc_state = rpc_status::PROCESS;
            return true;

        case rpc_status::PROCESS:
            spdlog::trace("[{}] rpc {} - {} (thread_id: {})", "PROCESS ", id, name, hasher(std::this_thread::get_id()));
            create_rpc();
            process_request();
            write_response();
            _rpc_state = rpc_status::FINISH;
            return true;

        case rpc_status::FINISH:
            spdlog::trace("[{}] rpc {} - {} (thread_id: {})", " FINISH ", id, name, hasher(std::this_thread::get_id()));
            return false;
    }

    return false;
}

}
