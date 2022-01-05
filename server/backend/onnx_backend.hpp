#pragma once

#include "backend_interface.hpp"

#include <memory>
#include <algorithm>
#include <numeric>
#include <string_view>
#include <vector>
#include <map>

namespace Ort
{
    struct Env;
    struct Session;
}

namespace xyz::backend
{
class onnx_backend : public backend_interface
{
    struct session_info
    {
        struct model_info
        {
            explicit model_info(const char* name, const std::vector<int64_t>& shape) : name{name}, shape{shape} { }
            const char* name;
            std::vector<int64_t> shape;
        };

        std::vector<model_info> inputs;
        std::vector<model_info> outputs;
        std::unique_ptr<Ort::Session> session;
    };

public:
    explicit onnx_backend() noexcept;
    ~onnx_backend();
    bool register_models(const std::vector<std::string_view>& models);
    infer_response infer(const infer_request& request) override;

private:
    std::unique_ptr<Ort::Env> _env;
    std::map<std::string_view, session_info> _model_sessions;

    template<typename T>
    T vector_product(const std::vector<T>& v)
    {
        return std::accumulate(v.begin(), v.end(), 1, std::multiplies<T>());
    }
};

}
