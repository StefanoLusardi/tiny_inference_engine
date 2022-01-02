#include <string>

namespace backend
{
class ServerBackend 
{
    virtual std::string say_hello() const = 0;
};
}
