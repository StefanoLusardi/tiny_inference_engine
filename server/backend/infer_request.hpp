#pragma once

#include <vector>
#include <string_view>

namespace xyz::backend
{
struct infer_request
{
    std::string_view model_name;
    std::vector<float> data;
};

}