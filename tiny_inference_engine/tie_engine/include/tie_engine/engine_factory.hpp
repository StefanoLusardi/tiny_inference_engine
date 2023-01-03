#pragma once

#include <tie_engine/engine_interface.hpp>
#include <memory>

namespace tie::engine
{
class engine_factory
{
public:
    virtual ~engine_factory() = default;
    static std::unique_ptr<engine_interface> create();
};
}
