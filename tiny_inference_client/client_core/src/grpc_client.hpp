#pragma once

#include <vector>
#include <memory>
#include <string>
#include <thread>
#include <functional>

#include <infer_request.hpp>
#include <infer_response.hpp>

#include <grpcpp/grpcpp.h>
#include <services.grpc.pb.h>

namespace tie::client_core
{
template<class ResponseTServerT>
struct AsyncClientBidiCall;

class grpc_client final
{
public:
    explicit grpc_client(const std::string& channel_address);
    ~grpc_client();

    void start_infer_stream();
    void stop_infer_stream();
    void send_infer_stream_request(const std::string& msg, bool is_last = false);
    void read_infer_stream_response();

    bool engine_ready_sync();
    void engine_ready_async();
    void engine_ready_async(const std::function<void(bool)>& callback);
    void set_engine_ready_callback(const std::function<void(bool)>& callback);

    bool load_model(const std::string& model_name) const;
    bool unload_model(const std::string& model_name) const;

    bool model_ready_sync();
    void model_ready_async();
    void set_model_ready_callback(const std::function<void(bool)>& callback);

    tie::infer_response infer_sync(const tie::infer_request& infer_request);
    void infer_async();
    void set_infer_callback(const std::function<void(bool)>& callback);

protected:
    std::function<void(bool)> _engine_ready_callback;
    std::function<void(bool)> _model_ready_callback;

private:
    void grpc_thread_worker(const std::shared_ptr<grpc::CompletionQueue>& cq);

    std::unique_ptr<tie::InferenceService::Stub> _stub;

    std::vector<std::thread> _async_grpc_threads;
    std::shared_ptr<grpc::CompletionQueue> _async_completion_queue;

    std::atomic_bool _is_bidi_stream_enabled;
    std::thread _bidi_grpc_thread;
    std::shared_ptr<grpc::CompletionQueue> _bidi_completion_queue;

    // TODO: use unique_ptr, but check for double delete.
    // AsyncClientBidiCall<tie::InferResponse>* _bidi_call = nullptr;

    // TODO: Enable configuration
    unsigned num_async_threads = 1u;
};

}