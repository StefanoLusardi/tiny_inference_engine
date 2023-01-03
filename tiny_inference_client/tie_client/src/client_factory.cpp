#include <tie_client/client_factory.hpp>
#include "grpc_client.hpp"

namespace tie::client
{
std::unique_ptr<client_interface> client_factory::create_client(const std::string& uri) 
{
	return std::make_unique<tie::client::grpc_client>(uri);
}

}
