#pragma once

#include <atomic>
#include <unordered_map>
#include <memory>

#include <services.grpc.pb.h>

#include <engine_interface.hpp>

#include "rpc.hpp"

#include <spdlog/spdlog.h>

namespace tie::server
{
class rpc_pool
{
public:
    ~rpc_pool()
    {
        spdlog::trace("delete rpc_pool");
        release();
    }

    static rpc_pool& get()
    {
        static rpc_pool instance;
        return instance;
    }

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

    void remove_rpc(uint64_t rpc_id)
    {
        // std::lock_guard<std::mutex> lock{ _rpcs_mutex };
        _rpcs.erase(rpc_id);
    }

    void release()
    {
        // _rpcs.clear();
    }
    
private:
    rpc_pool() = default;

    static std::atomic_uint64_t next_id;
    std::unordered_map<uint64_t, std::shared_ptr<rpc>> _rpcs {};
};

}
