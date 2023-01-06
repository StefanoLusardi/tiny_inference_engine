#pragma once

#include <map>
#include <string>

namespace tie
{
class data_type
{
public:
    enum Value : uint8_t
    {
        Bool,
        Uint8,
        Uint16,
        Uint32,
        Uint64,
        Int8,
        Int16,
        Int32,
        Int64,
        Fp16,
        Fp32,
        Fp64,
        String,
        Unknown,
    };

    data_type() = default;
    data_type(data_type::Value value) : value(value) {}
    data_type(const char* str_value) : value(from_string(str_value)) {}

    [[nodiscard]] data_type::Value from_string(const char* str_value);
    [[nodiscard]] const char* str() const;

    static std::map<data_type::Value, const char*> type_to_string;
    static std::map<const char*, data_type::Value> string_to_type;

    Value value = Value::Unknown;
};

}