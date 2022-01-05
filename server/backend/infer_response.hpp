#pragma once

#include <string_view>
#include <vector>

namespace xyz::backend
{
struct infer_response
{
    std::string_view model_name;
    std::vector<float> data;
    std::vector<int64_t> shape;
};

}