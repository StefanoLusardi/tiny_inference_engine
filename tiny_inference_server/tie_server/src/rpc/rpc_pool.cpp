#include "rpc_pool.hpp"

namespace tie::server
{
std::atomic_uint64_t rpc_pool::next_id = 0;

rpc_pool::~rpc_pool()
{
    spdlog::trace("delete rpc_pool");
    release();
}

rpc_pool& rpc_pool::get()
{
    static rpc_pool instance;
    return instance;
}

void rpc_pool::remove_rpc(uint64_t rpc_id)
{
    // std::lock_guard<std::mutex> lock{ _rpcs_mutex };
    _rpcs.erase(rpc_id);
}

void rpc_pool::release()
{
    _rpcs.clear();
}

}
