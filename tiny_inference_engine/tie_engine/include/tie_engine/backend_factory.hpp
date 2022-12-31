#pragma once

#include <memory>


namespace tie::engine
{
class backend_interface;

class backend_factory
{
public:
    enum class type
    {
        null,
        onnx,
    };

    virtual ~backend_factory() = default;
    static std::unique_ptr<backend_interface> create(const backend_factory::type backend_type);
};
}
