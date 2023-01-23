#pragma once

#include <tie_client/infer_parameters.hpp>
#include <tie_client/infer_tensor.hpp>

#include <vector>
#include <memory>
#include <string>

namespace tie::client
{
struct infer_request
{
    struct infer_request_output
    {
        std::string name;
        std::shared_ptr<infer_parameters> parameters;
    };

    std::string model_name;
    std::string model_version;
    std::string id;
    
    std::vector<infer_tensor> input_tensors;
    std::vector<infer_request_output> output_tensors;
    std::shared_ptr<infer_parameters> parameters;

    inline void add_input_tensor(void* data, const std::vector<uint64_t>& shape, data_type data_type, const std::string& name) 
    { 
        input_tensors.emplace_back(data, shape, data_type, name);
    }
};

}