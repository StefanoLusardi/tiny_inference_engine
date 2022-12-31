#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace tie::engine
{
struct infer_request
{
    std::string_view model_name;
    std::vector<uint8_t> data;
    std::vector<int64_t> shape;
};

}