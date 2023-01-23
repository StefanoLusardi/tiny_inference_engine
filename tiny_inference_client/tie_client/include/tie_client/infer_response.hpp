#pragma once

#include <tie_client/infer_parameters.hpp>
#include <tie_client/infer_tensor.hpp>

#include <string>
#include <memory>
#include <vector>

namespace tie::client
{
struct infer_response
{
    std::string model_name;
    std::string model_version;
    std::string id;
    std::shared_ptr<infer_parameters> parameters;
    std::vector<infer_tensor> output_tensors;

    void add_output_tensor(const infer_tensor &output)
    {
        output_tensors.push_back(output);
    }
};

}