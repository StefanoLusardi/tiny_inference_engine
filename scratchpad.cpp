#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <string_view>

template<typename T, typename Tensor>
constexpr auto getTensorContents(Tensor tensor)
{
    if constexpr (std::is_same_v<T, bool>) 
        return true;

    if constexpr (std::is_same_v<T, float>) 
        return 1.2345;
    
    if constexpr (std::is_same_v<T, const char*>) 
        return "asdfghj";
}

template<class T, typename Tensor>
constexpr void call_a(int i, const char* c, Tensor tensor)
{
    auto t = getTensorContents<T>(tensor);

    if constexpr (std::is_same_v<T, bool>)
        std::cout << "BOOL" << std::endl;

    if constexpr (std::is_same_v<T, float>) 
        std::cout << "FLOAT" << std::endl;
    
    if constexpr (std::is_same_v<T, const char*>) 
        std::cout << "CONST CHAR" << std::endl;
    
    std::cout << t << std::endl;
}

template<class T, typename Tensor>
constexpr void call_b(const std::string& s, bool b, Tensor tensor)
{
    auto t = getTensorContents<T>(tensor);

    if constexpr (std::is_same_v<T, bool>)
        std::cout << "BOOL" << std::endl;

    if constexpr (std::is_same_v<T, float>) 
        std::cout << "FLOAT" << std::endl;
    
    if constexpr (std::is_same_v<T, const char*>) 
        std::cout << "CONST CHAR" << std::endl;
    
    std::cout << t << std::endl;
}

/*
struct callable_t
{
    template<class T, typename Tensor>
    constexpr void operator()(int i, const char* c, Tensor tensor)
    {
        auto t = getTensorContents<T>(tensor);

        if constexpr (std::is_same_v<T, bool>)
            std::cout << "BOOL" << std::endl;

        if constexpr (std::is_same_v<T, float>) 
            std::cout << "FLOAT" << std::endl;
        
        if constexpr (std::is_same_v<T, const char*>) 
            std::cout << "CONST CHAR" << std::endl;
        
        std::cout << t << std::endl;
    }
};
*/

enum value : uint8_t
{
    BOOL,
    FLOAT,
    CONST_CHAR,
};

template<value v>
struct tv;

template<>
struct tv<value::BOOL>
{
    using type = bool;
};

template<>
struct tv<value::FLOAT>
{
    using type = float;
};

template<>
struct tv<value::CONST_CHAR>
{
    using type = const char*;
};

template<typename Tensor>
constexpr void simple_dispatch(value v, Tensor tensor)
{
    switch(v)
    {
        case BOOL:
            std::invoke(call_a<bool, decltype(tensor)>, 1, "some string", tensor);
            break;

        case FLOAT:
            std::invoke(call_a<float, decltype(tensor)>, 1, "some string", tensor);
            break;

        case CONST_CHAR:
            std::invoke(call_a<const char*, decltype(tensor)>, 1, "some string", tensor);
            break;
    }
}




template<typename T, typename Tensor>
struct f1
{
    constexpr void operator()(Tensor tensor, int i, const char* c)
    {
        auto t = getTensorContents<T>(tensor);

        if constexpr (std::is_same_v<T, bool>)
            std::cout << "BOOL" << std::endl;

        if constexpr (std::is_same_v<T, float>) 
            std::cout << "FLOAT" << std::endl;
        
        if constexpr (std::is_same_v<T, const char*>) 
            std::cout << "CONST CHAR" << std::endl;
        
        std::cout << t << std::endl;
    }
};

template<typename T, typename Tensor>
struct f2
{
    constexpr void operator()(Tensor tensor, const std::string& s, bool b)
    {
        auto t = getTensorContents<T>(tensor);

        if constexpr (std::is_same_v<T, bool>)
            std::cout << "BOOL" << std::endl;

        if constexpr (std::is_same_v<T, float>) 
            std::cout << "FLOAT" << std::endl;
        
        if constexpr (std::is_same_v<T, const char*>) 
            std::cout << "CONST CHAR" << std::endl;
        
        std::cout << t << std::endl;
    }
};

template<template<typename, typename> class func_t, typename Tensor, typename... Args>
constexpr void callable_wrapper1(value v, Tensor tensor, Args... args)
{
    switch(v)
    {
        case BOOL: return std::invoke(func_t<bool, Tensor>(), tensor, args...);
        case FLOAT: return std::invoke(func_t<float, Tensor>(), tensor, args...);
        case CONST_CHAR: return std::invoke(func_t<const char*, Tensor>(), tensor, args...);
    }
}

class MyTensorA{};
class MyTensorB{};

int main()
{
    MyTensorA tensor_a;
    value v_a = value::FLOAT;
    callable_wrapper1<f1>(v_a, tensor_a, 5, "");

    MyTensorB tensor_b;
    value v_b = value::CONST_CHAR;
    callable_wrapper1<f2>(v_b, tensor_b, "", false);

    return 0;
}