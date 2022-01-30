#include <client_core.hpp>

int main(int argc, char** argv)
{
    tie::client_core::client_core client("localhost:50051");

    return 0; //EXIT_SUCCESS;
}