#pragma once

#include <string_view>
#include "../backend/infer_request.hpp"
#include "../backend/infer_response.hpp"

namespace tie::engine
{
struct engine_interface
{
	virtual ~engine_interface() = default;
	virtual bool is_ready() const = 0;
    virtual bool load_model(const std::string_view& model_name) const = 0;
    virtual bool unload_model(const std::string_view& model_name) const = 0;
    virtual bool is_model_ready(const std::string_view& model_name) const = 0;
    virtual backend::infer_response infer(const backend::infer_request& request) = 0;
};
}