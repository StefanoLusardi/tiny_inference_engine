#include <tie_client/client_factory.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::level_enum::trace);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::info("Running client_example_model_infer");

    auto client = tie::client::client_factory::create_client("localhost:50051");

    std::vector<std::thread> workers;

    for (int i = 0; i < 4; ++i)
    {
        workers.emplace_back([&client, i]()
        {
            spdlog::debug("start infer {}", i);

            const auto [call_result, infer_response] = client->infer({});    
            if(call_result.ok())
            {
                spdlog::debug("infer OK");
            }
            else
            {
                spdlog::debug("infer ERROR");
            }
        });
    }

    for (auto&& w : workers)
        w.join();

    spdlog::info("Example client_example_model_infer finished");
    return EXIT_SUCCESS;
}
