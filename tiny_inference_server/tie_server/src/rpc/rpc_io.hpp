#pragma once

#include <grpcpp/server_context.h>
#include <grpcpp/support/async_unary_call.h>

namespace tie::server
{
template<class RequestT, class ResponseT>
struct rpc_io
{
   using io_request_t = RequestT;
   using io_response_t = ResponseT;

    explicit rpc_io() : response_writer{ &context }
    {
    }

    virtual ~rpc_io() = default;

    RequestT request;
    ResponseT response;
    grpc::ServerContext context;
    grpc::ServerAsyncResponseWriter<ResponseT> response_writer;
};

}
