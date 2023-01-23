#pragma once

#include <tie_client/data_type.hpp>
#include <tie_client/infer_response.hpp>
#include <tie_client/infer_request.hpp>
#include <services.grpc.pb.h>


namespace tie::client::util
{
    template <typename T, typename... Ts>
    struct is_any : std::disjunction<std::is_same<T, Ts>...> {};

    template <typename T, typename... Ts>
    inline constexpr bool is_any_v = is_any<T, Ts...>::value;
}

namespace tie::client
{
class tensor_converter
{
public:
    auto get_infer_request(const tie::client::infer_request& infer_request) const -> inference::ModelInferRequest;
    auto get_infer_response(const inference::ModelInferResponse& response) const -> tie::client::infer_response;

private:
    template<typename T, typename Tensor>
    constexpr static auto getTensorContents(Tensor* tensor)
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            if constexpr (std::is_const_v<Tensor>)
            {
                return tensor->contents().bool_contents().data();
            }
            else
            {
                return tensor->mutable_contents()->mutable_bool_contents();
            }
        }
        
        if constexpr (util::is_any_v<T, uint8_t, uint16_t, uint32_t>)
        {
            if constexpr (std::is_const_v<Tensor>)
            {
                return tensor->contents().uint_contents().data();
            }
            else
            {
                return tensor->mutable_contents()->mutable_uint_contents();
            }
        }
        
        if constexpr (std::is_same_v<T, uint64_t>)
        {
            if constexpr (std::is_const_v<Tensor>)
            {
                return tensor->contents().uint64_contents().data();
            }
            else
            {
                return tensor->mutable_contents()->mutable_uint64_contents();
            }
        }
        
        if constexpr (util::is_any_v<T, int8_t, int16_t, int32_t>)
        {
            if constexpr (std::is_const_v<Tensor>)
            {
                return tensor->contents().int_contents().data();
            }
            else
            {
                return tensor->mutable_contents()->mutable_int_contents();
            }
        }

        if constexpr (std::is_same_v<T, int64_t>)
        {
            if constexpr (std::is_const_v<Tensor>)
            {
                return tensor->contents().int64_contents().data();
            }
            else
            {
                return tensor->mutable_contents()->mutable_int64_contents();
            }
        }
        
        if constexpr (util::is_any_v<T, float>) // fp16, 
        {
            if constexpr (std::is_const_v<Tensor>)
            {
                return tensor->contents().fp32_contents().data();
            }
            else
            {
                return tensor->mutable_contents()->mutable_fp32_contents();
            }
        }
        
        if constexpr (std::is_same_v<T, double>)
        {
            if constexpr (std::is_const_v<Tensor>)
            {
                return tensor->contents().fp64_contents().data();
            }
            else
            {
                return tensor->mutable_contents()->mutable_fp64_contents();
            }
        }
        
        if constexpr (std::is_same_v<T, char>)
        {
            if constexpr (std::is_const_v<Tensor>)
            {
                return tensor->contents().bytes_contents().data();
            }
            else
            {
                return tensor->mutable_contents()->mutable_bytes_contents();
            }
        }

        // static_assert(!sizeof(T), "Invalid type to AddDataToTensor");
    }

    template<typename T, typename Tensor>
    struct tensor_data_writer
    {
        constexpr void operator()(Tensor* tensor, const void* source_data, size_t size) const
        {
            const auto data = static_cast<const T*>(source_data);
            auto contents = tie::client::tensor_converter::getTensorContents<T>(tensor);

            if constexpr (std::is_same_v<T, char>)
            {
                contents->Add(data);
            }
            // else if constexpr (std::is_same_v<T, fp16>)
            // {
            //     for (auto i = 0U; i < size; ++i)
            //     {
            //         contents->Add(static_cast<float>(data[i]));
            //     }
            // }
            else
            {
                for (auto i = 0U; i < size; ++i)
                {
                    contents->Add(data[i]);
                }
            }
        }
    };

    template<typename T, typename Tensor>
    struct tensor_data_reader
    {
        /*constexpr*/ void operator()(Tensor* tensor, infer_tensor* output, size_t size) const
        {
            const auto bytes_to_copy = size * sizeof(T);
            std::vector<std::byte> data;
            data.resize(bytes_to_copy);
            const auto contents = tie::client::tensor_converter::getTensorContents<T>(tensor);
            
            if constexpr (std::is_same_v<T, char>)
            {
                std::memcpy(data.data(), contents, size * sizeof(std::byte));
                output->data = std::move(data.data());
                // output->data = data.data();
                return;
            }
            
            if constexpr (util::is_any_v<T, int8_t, uint8_t, int16_t, uint16_t>) // fp16
            {
                for (auto i = 0U; i < size; ++i)
                {
                    std::memcpy(&(data[i * sizeof(T)]), &(contents[i]), sizeof(T));
                }
                output->data = std::move(data.data());
                // output->data = data.data();
                return;
            }

            std::memcpy(data.data(), contents, bytes_to_copy);
            output->data = std::move(data.data());
            // output->data = data.data();
        }
    };

    template<template<typename, typename> class func_t, typename TensorT, typename... Args>
    /*constexpr*/ void tensor_data_converter_call_wrapper(tie::client::data_type type, TensorT* tensor, Args&&... args) const
    {
        switch(type)
        {
            case data_type::Bool:   return std::invoke(func_t<bool, TensorT>(), tensor, args...);
            case data_type::Uint8:  return std::invoke(func_t<uint8_t, TensorT>(), tensor, args...);
            case data_type::Uint16: return std::invoke(func_t<uint16_t, TensorT>(), tensor, args...);
            case data_type::Uint32: return std::invoke(func_t<uint32_t, TensorT>(), tensor, args...);
            case data_type::Uint64: return std::invoke(func_t<uint64_t, TensorT>(), tensor, args...);
            case data_type::Int8:   return std::invoke(func_t<int8_t, TensorT>(), tensor, args...);
            case data_type::Int16:  return std::invoke(func_t<int16_t, TensorT>(), tensor, args...);
            case data_type::Int32:  return std::invoke(func_t<int32_t, TensorT>(), tensor, args...);
            case data_type::Int64:  return std::invoke(func_t<int64_t, TensorT>(), tensor, args...);
            case data_type::Fp16:   return; // std::invoke(func_t<uint64_t, TensorT>(), tensor, args...);
            case data_type::Fp32:   return std::invoke(func_t<float, TensorT>(), tensor, args...);
            case data_type::Fp64:   return std::invoke(func_t<double, TensorT>(), tensor, args...);
            case data_type::String: return std::invoke(func_t<char, TensorT>(), tensor, args...);
            case data_type::Unknown: static_assert("Unknown DataType"); return;
        }
    }
};

}
