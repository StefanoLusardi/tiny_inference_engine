#include "onnx_backend.hpp"
#include "infer_response.hpp"
#include "onnxruntime_c_api.h"
#include <cstdint>

#include <spdlog/spdlog.h>
#include <vector>
// #include <spdlog/fmt/ostr.h>

namespace tie::backend
{
onnx_backend::onnx_backend() noexcept
    : _env{ std::make_unique<Ort::Env>(OrtLoggingLevel::ORT_LOGGING_LEVEL_WARNING, "onnxruntime_backend") }
{
}

onnx_backend::~onnx_backend() {}

bool onnx_backend::load_models(const std::vector<std::string_view>& models)
{
    Ort::SessionOptions session_options;

#if defined(WITH_CUDA)
    OrtCUDAProviderOptions cuda_options;
    cuda_options.device_id = 0;
    session_options.AppendExecutionProvider_CUDA(cuda_options);
#endif

    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    session_options.SetIntraOpNumThreads(1);
    session_options.SetInterOpNumThreads(1);

    Ort::AllocatorWithDefaultOptions allocator;

    for (auto model_name : models)
    {
        onnx_backend::session_info session_info;

        auto model_name_str = std::basic_string<ORTCHAR_T>(model_name.begin(), model_name.end());
        auto session = std::make_unique<Ort::Session>(*_env, model_name_str.c_str(), session_options);

        for (auto idx = 0; idx < session->GetInputCount(); ++idx)
        {
            const auto name = session->GetInputName(idx, allocator);
            const auto shape = session->GetInputTypeInfo(idx).GetTensorTypeAndShapeInfo().GetShape();
            const auto type = session->GetInputTypeInfo(idx).GetTensorTypeAndShapeInfo().GetElementType();
            session_info.inputs.emplace_back(name, shape, type);
        }

        for (auto idx = 0; idx < session->GetOutputCount(); ++idx)
        {
            const auto name = session->GetOutputName(idx, allocator);
            const auto shape = session->GetOutputTypeInfo(idx).GetTensorTypeAndShapeInfo().GetShape();
            const auto type = session->GetOutputTypeInfo(idx).GetTensorTypeAndShapeInfo().GetElementType();
            session_info.outputs.emplace_back(name, shape, type);
        }

        session_info.session = std::move(session);
        _model_sessions.emplace(model_name, std::move(session_info));
    }

    return true;
}

infer_response onnx_backend::infer(const infer_request& request)
{
    const auto request_model = _model_sessions.find(request.model_name);
    if (request_model == _model_sessions.end())
    {
        spdlog::warn("Model {} not registered", request.model_name);
        return {};
    }

    const auto vector_product = [](const std::vector<int64_t>& v) -> int64_t { return std::accumulate(v.begin(), v.end(), 1ll, std::multiplies<int64_t>()); };

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

    std::vector<const char*> input_names;
    std::vector<Ort::Value> input_data;
    int idx = 0;
    for (auto idx = 0; idx < request_model->second.inputs.size(); ++idx)
    {
        const auto input_name = request_model->second.inputs[idx].name;
        input_names.push_back(input_name);

        const auto input_type = request_model->second.inputs[idx].type;

        const auto input_shape = request_model->second.inputs[idx].shape;
        const auto input_tensor_size = vector_product(input_shape);

        input_data.push_back(Ort::Value::CreateTensor(memory_info, (void*)request.data.data(), 4 * input_tensor_size, input_shape.data(), input_shape.size(), input_type));
    }

    std::vector<const char*> output_names;
    std::vector<Ort::Value> output_data;
    for (auto idx = 0; idx < request_model->second.outputs.size(); ++idx)
    {
        const auto output_name = request_model->second.outputs[idx].name;
        output_names.push_back(output_name);

        output_data.emplace_back(nullptr);
    }

    const auto session = request_model->second.session.get();
    session->Run(Ort::RunOptions{ nullptr }, input_names.data(), input_data.data(), input_data.size(), output_names.data(), output_data.data(), output_data.size());

    infer_response response;

    for (auto idx = 0; idx < request_model->second.outputs.size(); ++idx)
    {
        void** tensor_data = output_data[idx].GetTensorMutableData<void*>();
        size_t tensor_count = output_data[idx].GetTensorTypeAndShapeInfo().GetElementCount();
        std::vector<int64_t> tensor_shape = output_data[idx].GetTensorTypeAndShapeInfo().GetShape();

        infer_response::tensor_info tensor_info{ tensor_data, tensor_count, tensor_shape, data_type::dt_float_dt };
        response.tensors.emplace(output_names[idx], std::move(tensor_info));
    }

    return response;
}

/*
    Ort::AllocatorWithDefaultOptions allocator;
    for (auto [model_name, model_session] : _model_sessions)
    {
        // spdlog::debug("Model: {}", model_name.data());

        const auto num_inputs = model_session->GetInputCount();
        for(auto input_idx = 0; input_idx < num_inputs; ++input_idx)
        {
            const auto input_name = model_session->GetInputName(input_idx, allocator);
            const auto input_info = model_session->GetInputTypeInfo(input_idx).GetTensorTypeAndShapeInfo();

            spdlog::debug("  Input {}: {}", input_idx, input_name);
            // See: https://fmt.dev/latest/api.html#format-api
            // spdlog::debug("    Shape: {}", input_info.GetShape());
            // spdlog::debug("    Type: {}", input_info.GetElementType());
        }

        const auto num_outputs = model_session->GetOutputCount();
        for(auto output_idx = 0; output_idx < num_outputs; ++output_idx)
        {
            const auto output_name = model_session->GetOutputName(output_idx, allocator);
            const auto output_info = model_session->GetOutputTypeInfo(output_idx).GetTensorTypeAndShapeInfo();

            spdlog::debug("  Output {}: {}", output_idx, output_name);
            
            // See: https://fmt.dev/latest/api.html#format-api
            // spdlog::debug("    Shape: {}", output_info.GetShape());
            // spdlog::debug("    Type: {}", output_info.GetElementType());
        }
    }
*/

}
