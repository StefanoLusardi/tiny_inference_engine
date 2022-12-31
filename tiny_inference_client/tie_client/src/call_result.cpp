#include <tie_client/call_result.hpp>

namespace tie::client
{
call_result::call_result(call_result_code code) noexcept : _code { code }, _error_message{ std::string("") }
{
}

call_result::call_result(call_result_code code, const std::string& error_message) noexcept : _code { code }, _error_message{ error_message }
{
}
    
std::string call_result::error_message() const 
{ 
    return _error_message;
}

bool call_result::ok() const 
{ 
    return _code == call_result_code::CALL_OK;
}

const call_result& call_result::OK = call_result(call_result_code::CALL_OK);

const call_result& call_result::ERROR = call_result(call_result_code::CALL_ERROR, std::string("Unknown error"));

}

