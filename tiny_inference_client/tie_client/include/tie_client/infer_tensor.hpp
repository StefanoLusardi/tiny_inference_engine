#pragma once

#include <tie_client/infer_parameters.hpp>
#include <tie_client/data_type.hpp>

#include <vector>
#include <memory>
#include <string>

namespace tie::client
{
struct infer_tensor
{
    infer_tensor()
    {
    }

    infer_tensor(
        void *data,
        std::vector<uint64_t> shape,
        data_type dataType,
        std::string name)
    : datatype(dataType) 
    {
        this->data = data;
        this->shape = std::move(shape);
        this->name = std::move(name);
        this->parameters = std::make_unique<RequestParameters>();
    }

    std::string name;
    std::vector<uint64_t> shape;
    data_type datatype;
    std::shared_ptr<RequestParameters> parameters;
    void* data;
};

}