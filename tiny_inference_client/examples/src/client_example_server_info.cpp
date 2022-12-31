#include <tie_client/client_factory.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::level_enum::trace);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::info("Running client_example_server_info");

    auto client = tie::client::client_factory::create_client("localhost:50051");

    // Server Live
    {
        const auto [call_result, is_server_live] = client->is_server_live();    
        if(call_result.ok())
        {
            spdlog::debug("is_server_live: {} ", is_server_live);
        }
        else
        {
            spdlog::debug("ERROR on call is_server_live");
        }
    }
    
    // Server Ready
    {
        const auto [call_result, is_server_ready] = client->is_server_ready();    
        if(call_result.ok())
        {
            spdlog::debug("is_server_ready: {} ", is_server_ready);
        }
        else
        {
            spdlog::debug("ERROR on call is_server_ready");
        }
    }

    // Server Metadata
    {
        const auto [call_result, server_metadata] = client->server_metadata();    
        if(call_result.ok())
        {
            spdlog::debug("server_metadata: ");
            // server_metadata
        }
        else
        {
            spdlog::debug("ERROR on call server_metadata");
        }
    }

    spdlog::info("Example client_example_server_info finished");
    return EXIT_SUCCESS;
}
