#pragma once

#include "backend_interface.hpp"
#include <memory>

#if defined(_WIN32) || defined(_WIN64)
#define API_EXPORT_TIE_SERVER __declspec(dllexport)
#else
#define API_EXPORT_TIE_SERVER __attribute__((visibility("default")))
#endif

namespace tie::backend
{
class API_EXPORT_TIE_SERVER backend_factory
{
public:
    virtual ~backend_factory() = default;
    static std::unique_ptr<tie::backend::backend_interface> create(const tie::backend::type backend_type);
};
}
