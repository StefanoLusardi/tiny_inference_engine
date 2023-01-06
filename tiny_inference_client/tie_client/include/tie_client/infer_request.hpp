#pragma once

#include <tie_client/infer_parameters.hpp>
#include <tie_client/infer_tensor.hpp>

#include <vector>
#include <memory>
#include <string>

namespace tie::client
{
struct InferenceRequestOutput
{
    std::string name;
    std::shared_ptr<RequestParameters>parameters;
    void *data_;
};

using InferenceRequestInput = infer_tensor;

struct infer_request
{
    std::string model_name;
    std::string model_version;
    std::string id;

    std::shared_ptr<RequestParameters> parameters;
    std::vector<InferenceRequestInput> inputs;
    std::vector<InferenceRequestOutput> outputs;

    void addInputTensor(void* data, const std::vector<uint64_t>& shape, data_type data_type, const std::string& name) 
    { 
        inputs.emplace_back(data, shape, data_type, name);
    }
};

}