#include "rpc_pool.hpp"


namespace tie::server
{
std::atomic_uint64_t rpc_pool::next_id = 0;

}
