#include <client_core.hpp>
#include <ios>
#include <iostream>

int main(int argc, char** argv)
{
    tie::client_core::client_core client("localhost:50051");

    const bool is_engine_ready_sync = client.engine_ready_sync();
    std::cout << "is_engine_ready (sync call): " << std::boolalpha << is_engine_ready_sync << std::endl;

    const auto engine_ready_callback = [](bool is_ready){ std::cout << "is_engine_ready (async call): " << std::boolalpha << is_ready << std::endl; };
    client.set_engine_ready_callback(engine_ready_callback);
    client.engine_ready_async();

    return EXIT_SUCCESS;
}