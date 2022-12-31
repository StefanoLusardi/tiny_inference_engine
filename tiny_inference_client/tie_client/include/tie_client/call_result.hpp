#pragma once

#include <string>

namespace tie::client
{
enum call_result_code { CALL_OK, CALL_ERROR };

class call_result
{
public:
    explicit call_result(call_result_code code) noexcept;
    explicit call_result(call_result_code code, const std::string& error_message) noexcept;
    
    bool ok() const;
    std::string error_message() const;

    static const call_result& OK;
    static const call_result& ERROR;

private:
    const call_result_code _code;
    const std::string _error_message;
};

}
