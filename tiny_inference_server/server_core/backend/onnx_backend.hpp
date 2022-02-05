#pragma once

#include "backend_interface.hpp"

#include <memory>
#include <algorithm>
#include <string_view>
#include <vector>
#include <map>

#include <onnxruntime_cxx_api.h>

// #if defined(WIN32)
// #pragma push(disable : C4244)
// #endif

#include <numeric>

// #if defined(WIN32)
// #pragma pop
// #endif

namespace tie::backend
{

template<ONNXTensorElementDataType>
struct GetTensorType;

template<>
struct GetTensorType<ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT>
{
    using type = float;
};

template<>
struct GetTensorType<ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8>
{
    using type = uint8_t;
};

class onnx_backend : public backend_interface
{
    struct session_info
    {
        struct model_info
        {
            explicit model_info(const char* name, const std::vector<int64_t>& shape, ONNXTensorElementDataType type)
                : name{ name }
                , shape{ shape }
                , type{ type }
            {
            }
            const char* name;
            std::vector<int64_t> shape;
            ONNXTensorElementDataType type;
        };

        std::vector<model_info> inputs;
        std::vector<model_info> outputs;
        std::unique_ptr<Ort::Session> session;
    };

public:
    explicit onnx_backend() noexcept;
    ~onnx_backend();
    bool load_models(const std::vector<std::string_view>& models) override;
    infer_response infer(const infer_request& request) override;

private:
    std::unique_ptr<Ort::Env> _env;
    std::map<std::string_view, session_info> _model_sessions;
};

}
