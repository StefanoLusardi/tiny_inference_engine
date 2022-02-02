#pragma once

#include "backend_interface.hpp"
#include <memory>

namespace tie::backend
{
class backend_factory
{
public:
    virtual ~backend_factory() = default;
    static std::unique_ptr<tie::backend::backend_interface> create(const tie::backend::type backend_type);
};
}
