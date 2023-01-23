#pragma once

#include <map>
#include <string>

namespace tie::client
{
class data_type
{
public:
    enum value : uint8_t
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

    data_type() : _value{value::Unknown} {}
    data_type(data_type::value value) : _value(value) {}
    data_type(const std::string& str_value) : _value(from_string(str_value)) {}

    constexpr operator value() const { return _value; }

    [[nodiscard]] data_type::value from_string(const std::string& str_value);
    [[nodiscard]] std::string str() const;

private:
    value _value = value::Unknown;
    static std::map<data_type::value, std::string> _type_to_string;
    static std::map<std::string, data_type::value> _string_to_type;
};

}