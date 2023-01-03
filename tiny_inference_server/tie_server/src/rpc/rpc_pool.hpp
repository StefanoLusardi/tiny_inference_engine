#pragma once

#include <tie_engine/engine_interface.hpp>
#include "rpc.hpp"

#include <services.grpc.pb.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <unordered_map>
#include <memory>

namespace tie::server
{
class rpc_pool
{
public:
    ~rpc_pool();
    static rpc_pool& get();

    void remove_rpc(uint64_t rpc_id);
    void release();

    template<class T>
    std::shared_ptr<rpc> create_rpc(
        const std::shared_ptr<inference::GRPCInferenceService::AsyncService>& service,
        const std::shared_ptr<grpc::ServerCompletionQueue>& cq,
        const std::shared_ptr<engine::engine_interface>& engine)
    {
        // assert std::is_base_of_v<T, rpc>

        const auto rpc_id = next_id++;
        const auto rpc = std::make_shared<T>(rpc_id, service, cq, engine);

        // std::lock_guard<std::mutex> lock{ _rpcs_mutex };
        _rpcs[rpc_id] = rpc;
        return rpc;
    }
    
private:
    rpc_pool() = default;

    static std::atomic_uint64_t next_id;
    std::unordered_map<uint64_t, std::shared_ptr<rpc>> _rpcs {};
};

}
