#pragma once

#include <variant>
#include <string>
#include <map>

namespace tie
{
using Parameter = std::variant<bool, int32_t, double, std::string>;

struct RequestParameters
{
private:
    std::map<std::string, Parameter> parameters;
};

}