#pragma once

#include "engine_interface.hpp"

#include <memory>

namespace xyz::backend
{
    class backend_interface;
}

namespace xyz::engine
{
class engine final : public engine_interface
{
public:
    explicit engine();
    ~engine() override;

private:
    std::unique_ptr<xyz::backend::backend_interface> _backend;
};
}