#include "tensor_converter.hpp"
#include <numeric>

namespace tie::client
{

auto tensor_converter::get_infer_request(const tie::client::infer_request& infer_request) const -> inference::ModelInferRequest
{
    inference::ModelInferRequest request;

    request.set_model_name(infer_request.model_name);
    request.set_model_version(infer_request.model_version);
    request.set_id(infer_request.id);

    for (const auto& request_input : infer_request.input_tensors)
    {
        auto tensor = request.add_inputs();
        tensor->set_name(request_input.name);
        tensor->set_datatype(request_input.datatype.str());

        for (auto&& shape : request_input.shape)
        {
            tensor->add_shape(shape);
        }

        const size_t input_size = std::accumulate(request_input.shape.begin(), request_input.shape.end(), 1, std::multiplies<>());
        tensor_data_converter_call_wrapper<tensor_data_writer>(request_input.datatype, tensor, request_input.data, input_size);
    }

    return std::move(request);
}

auto tensor_converter::get_infer_response(const inference::ModelInferResponse& response) const -> tie::client::infer_response
{
    tie::client::infer_response infer_response;

    infer_response.model_name = response.model_name();
    infer_response.model_version = response.model_version();
    infer_response.id = response.id();

    for (const auto& response_output : response.outputs())
    {
        tie::client::infer_tensor infer_tensor_output;

        infer_tensor_output.name = response_output.name();
        infer_tensor_output.datatype = tie::client::data_type(response_output.datatype());
        infer_tensor_output.shape.reserve(response_output.shape_size());
        
        for (auto&& shape : response_output.shape())
        {
            infer_tensor_output.shape.push_back(static_cast<size_t>(shape));
        }

        const size_t output_size = std::accumulate(response_output.shape().begin(), response_output.shape().end(), size_t(1), std::multiplies<>());
        tensor_data_converter_call_wrapper<tensor_data_reader>(infer_tensor_output.datatype, const_cast<inference::ModelInferResponse_InferOutputTensor*>(&response_output), &infer_tensor_output, output_size);
        infer_response.add_output_tensor(infer_tensor_output);
    }

    return std::move(infer_response);
}

}