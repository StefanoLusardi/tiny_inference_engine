#pragma once

#include <variant>
#include <string>
#include <map>

namespace tie::client
{
using parameter = std::variant<bool, int32_t, double, std::string>;

struct infer_parameters
{
private:
    std::map<std::string, parameter> parameters;
};

}