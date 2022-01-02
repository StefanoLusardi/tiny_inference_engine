#include <string>
#include "../backend.hpp"

namespace backend
{
class OnnxhBackend : ServerBackend
{
    std::string say_hello() const override 
    {
        return "Onnx Backend!";
    }
};
}
