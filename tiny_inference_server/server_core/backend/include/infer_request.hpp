#pragma once

#include <string_view>
#include <vector>

namespace tie::backend
{
struct infer_request
{
    std::string_view model_name;
    std::vector<float> data;
    std::vector<int64_t> shape;
};

}