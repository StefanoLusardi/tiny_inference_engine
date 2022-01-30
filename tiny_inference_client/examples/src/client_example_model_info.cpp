#include <atomic>
#include <client_core.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "Running client_example_model_info" << "\n" << std::endl;

    tie::client_core::client_core client("localhost:50051");

    const bool is_engine_ready_sync1 = client.engine_ready_sync();
    const bool is_engine_ready_sync2 = client.engine_ready_sync();
    const bool is_engine_ready_sync3 = client.engine_ready_sync();
    const bool is_engine_ready_sync4 = client.engine_ready_sync();

    std::cout << "is_engine_ready (sync call 1): " << std::boolalpha << is_engine_ready_sync1 << "\n" << std::endl;
    std::cout << "is_engine_ready (sync call 2): " << std::boolalpha << is_engine_ready_sync2 << "\n" << std::endl;
    std::cout << "is_engine_ready (sync call 3): " << std::boolalpha << is_engine_ready_sync3 << "\n" << std::endl;
    std::cout << "is_engine_ready (sync call 4): " << std::boolalpha << is_engine_ready_sync4 << "\n" << std::endl;

    std::atomic_int i = 1;
    const auto engine_ready_callback = [&i](bool is_ready)
    { 
        std::cout << "is_engine_ready (async call " << i++ << "): " << std::boolalpha << is_ready << "\n" << std::endl; 
    };

    client.set_engine_ready_callback(engine_ready_callback);
    client.engine_ready_async();
    client.engine_ready_async();
    client.engine_ready_async();
    client.engine_ready_async();

    i = 100;
    client.engine_ready_async([&i](bool is_ready){
        std::cout << "is_engine_ready (async call " << i++ << "): " << std::boolalpha << is_ready << "\n" << std::endl; 
    });

    std::cout << "Example client_example_model_info finished" << "\n" << std::endl;
    
    return EXIT_SUCCESS;
}