#pragma once

#include <tie_client/infer_parameters.hpp>
#include <tie_client/infer_tensor.hpp>

#include <string>
#include <memory>
#include <vector>

namespace tie
{
using InferenceResponseOutput = infer_tensor;

struct infer_response
{
    std::string model_name;
    std::string model_version;
    std::string id;

    std::shared_ptr<RequestParameters> parameters;
    std::vector<InferenceResponseOutput> outputs;

    void addOutput(const InferenceResponseOutput &output)
    {
        this->outputs.push_back(output);
    }
};

}