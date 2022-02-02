#include "backend_factory.hpp"
#include "infer_request.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    auto backend = tie::backend::backend_factory::create(tie::backend::type::onnx);
    
    auto request = tie::backend::infer_request();
    auto response = backend->infer(request);
    
    return EXIT_SUCCESS;
}