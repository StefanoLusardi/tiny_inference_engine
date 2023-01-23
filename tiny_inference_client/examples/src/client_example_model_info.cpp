#include <tie_client/client_factory.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::level_enum::trace);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::info("Running client_example_model_info");

    auto client = tie::client::client_factory::create_client("localhost:50051");

    auto model_name = "model_name";
    auto model_version = "v";

    // Model Ready
    {
        const auto [call_result, is_model_ready] = client->is_model_ready(model_name, model_version);
        if(call_result.ok())
        {
            spdlog::debug("is_model_ready: {}", is_model_ready);
        }
        else
        {
            spdlog::debug("ERROR on call is_model_ready");
        }
    }

    // Model List
    {
        const auto [call_result, model_list] = client->model_list();    
        if(call_result.ok())
        {
            spdlog::debug("model_list:");
            for (auto&& model : model_list)
                spdlog::debug(model);
        }
        else
        {
            spdlog::debug("ERROR on call model_list");
        }
    }

    // Model Load
    {
        const auto [call_result, is_model_loaded] = client->model_load(model_name, model_version);
        if(call_result.ok())
        {
            spdlog::debug("model_load: {}", is_model_loaded);
        }
        else
        {
            spdlog::debug("ERROR on call model_load");
        }
    }

    // Model Unload
    {
        const auto [call_result, is_model_unloaded] = client->model_unload(model_name, model_version);
        if(call_result.ok())
        {
            spdlog::debug("model_unload: {}", is_model_unloaded);
        }
        else
        {
            spdlog::debug("ERROR on call model_unload");
        }
    }

    // Model Metadata
    {
        const auto [call_result, model_metadata] = client->model_metadata(model_name, model_version);
        if(call_result.ok())
        {
            spdlog::debug("model_metadata: ");
            // model_metadata
        }
        else
        {
            spdlog::debug("ERROR on call model_metadata");
        }
    }

    spdlog::info("Example client_example_model_info finished");
    return EXIT_SUCCESS;
}
