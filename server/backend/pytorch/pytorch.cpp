#include <string>
#include "../backend.hpp"

namespace backend
{
class PytorchBackend : ServerBackend
{
    std::string say_hello() const override
    {
        return "Pytorch Backend!";
    }
};
}
