#pragma once

#include <services.grpc.pb.h>
#include <tie_engine/engine_interface.hpp>
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
        const std::shared_ptr<tie::engine::engine_interface>& engine);
    virtual ~rpc_unary_async() override;
    
    bool execute() override;

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
