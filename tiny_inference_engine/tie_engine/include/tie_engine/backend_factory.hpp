#pragma once

#include <tie_engine/backend_type.hpp>
#include <tie_engine/backend_interface.hpp>
#include <memory>

namespace tie::engine
{
class backend_factory
{
public:
    ~backend_factory() = default;
    static std::unique_ptr<backend_interface> create(backend_type backend_type);

private:
    backend_factory() = default;
};

}
