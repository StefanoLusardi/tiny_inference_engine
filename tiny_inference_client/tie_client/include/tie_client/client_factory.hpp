#pragma once

#include <tie_client/client_interface.hpp>
#include <memory>

namespace tie::client
{

class client_factory
{
public:
    ~client_factory() = default;

    static std::unique_ptr<client_interface> create_client(const std::string& uri);
    
private:    
    client_factory() = default;
};

}

