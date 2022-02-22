#pragma once

#include <string_view>
#include <vector>
#include <map>

namespace tie::backend
{
enum data_type
{
    dt_float_dt,
    dt_uint8
};

struct infer_response
{
    struct tensor_info
    {
        void* data;
        size_t count;
        std::vector<int64_t> shape;
        data_type type;
    };

    std::map<std::string_view, tensor_info> tensors;
    std::string_view model_name;
};

}