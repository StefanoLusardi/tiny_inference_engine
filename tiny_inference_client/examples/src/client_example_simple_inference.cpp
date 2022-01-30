#include <iostream>

#include <client_core.hpp>

int main(int argc, char** argv)
{
    std::cout << "Running client_example_model_info" << "\n" << std::endl;
    tie::client_core::client_core client("localhost:50051");

    // client.infer

    return EXIT_SUCCESS;
}