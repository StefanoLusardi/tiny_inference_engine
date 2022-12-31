#pragma once

#include <services.grpc.pb.h>
#include <engine_interface.hpp>
#include "rpc.hpp"

#include <spdlog/spdlog.h>
#include "rpc_pool.hpp"

namespace tie::server
{
class rpc_unary_async : public tie::server::rpc
{
public:
    rpc_unary_async(
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

    virtual ~rpc_unary_async() override
    {
        spdlog::trace("delete rpc {} - {}", id, name);
    }

    bool execute() override
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

protected:
    virtual void setup_request() = 0;
    virtual void create_rpc() = 0;
    virtual void process_request() = 0;
    virtual void write_response() = 0;

    std::shared_ptr<inference::GRPCInferenceService::AsyncService> _service;
    std::shared_ptr<grpc::ServerCompletionQueue> _cq;
    std::shared_ptr<tie::engine::engine_interface> _engine;

private:
    enum class rpc_status : char
    {
        CREATE,
        PROCESS,
        FINISH
    };
    rpc_status _rpc_state;
};

}
