#pragma once

#include <string>
#include <vector>

namespace tie::client
{
struct model_metadata
{
    struct tensor_metadata
    {
        std::string name;
        std::string datatype;
        std::vector<int64_t> shape;
    };

    std::string model_name;
    std::vector<std::string> model_versions;
    std::string platform;
    std::vector<tensor_metadata> inputs;
    std::vector<tensor_metadata> outputs;
};

}