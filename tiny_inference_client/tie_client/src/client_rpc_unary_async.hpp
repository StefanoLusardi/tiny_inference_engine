#pragma once

#include <grpcpp/client_context.h>
#include <functional>

namespace tie::client
{

struct AsyncClientCall
{
    grpc::ClientContext context;
    grpc::Status result_code;
    virtual bool proceed(bool ok) = 0;
};

template<class ResponseT>
struct AsyncClientCallback : public AsyncClientCall
{
    ResponseT response;
    void set_response_callback(std::function<void(ResponseT)> response_callback) { _on_response_callback = response_callback; }

protected:
    std::function<void(ResponseT)> _on_response_callback;
};

template<class ResponseT>
struct AsyncClientUnaryCall : public AsyncClientCallback<ResponseT>
{
    std::unique_ptr<grpc::ClientAsyncResponseReader<ResponseT>> rpc;

    bool proceed(bool ok) override
    {
        if (ok)
        {
            if (this->result_code.ok())
            {
                // SPDLOG_INFO(this->response.message());
                if (this->_on_response_callback) this->_on_response_callback(this->response);
            }
            else
            {
                SPDLOG_INFO("RPC failed: (" << this->result_code.error_code() << ") " << this->result_code.error_message());
            }
        }

        return false;
    }
};

}
