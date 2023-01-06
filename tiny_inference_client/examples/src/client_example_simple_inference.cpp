#include <spdlog/spdlog.h>
#include <opencv2/opencv.hpp>

#include <tie_client/client_factory.hpp>
#include <tie_client/infer_request.hpp>
#include <tie_client/infer_response.hpp>

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::level_enum::trace);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::info("Running client_example_simple_inference");

    auto client = tie::client::client_factory::create_client("localhost:50051");

    std::string image_file_path{ "images/dog.jpg" };
    cv::Mat image = cv::imread(image_file_path, cv::ImreadModes::IMREAD_COLOR);
    cv::imshow("Input image", image);
    // cv::waitKey(0);
    
    cv::resize(image, image, cv::Size(224, 224), cv::InterpolationFlags::INTER_CUBIC);
    cv::cvtColor(image, image, cv::ColorConversionCodes::COLOR_BGR2RGB);
    image.convertTo(image, CV_32F, 1.0 / 255);
    cv::imshow("Resized image", image);
    // cv::waitKey(0);

    auto request = tie::client::infer_request();
    request.model_name = "models/squeezenet1.1-7.onnx";
    request.model_version = "";
    client->model_load(request.model_name, request.model_version);

    std::vector<uint64_t> shape {static_cast<uint64_t>(image.cols), static_cast<uint64_t>(image.rows), static_cast<uint64_t>(image.channels())};
    request.addInputTensor(image.data, shape, tie::client::data_type::Fp32, "squeezenet0_flatten0_reshape0");
    auto [call_result, response] = client->infer(request);

    if (!call_result.ok())
    {
        spdlog::error("Unable to perform inference");
        return EXIT_FAILURE;
    }

    auto result = response.outputs;

    return EXIT_SUCCESS;
}