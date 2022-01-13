#pragma once

#include <vector>
#include <memory>
#include <string>
#include <thread>

// TODO: check includes
#include <grpcpp/grpcpp.h>
#include <services.grpc.pb.h>

namespace tie::client_core
{
template<class ResponseTServerT>
struct AsyncClientBidiCall;

class grpc_client final
{
public:
	explicit grpc_client(const std::string &channel_address);
	~grpc_client();

	void start_infer_stream();
	void stop_infer_stream();
	void send_infer_stream_request(const std::string &msg, bool is_last = false);
	void read_infer_stream_response();
	
	bool engine_ready_sync();
	void engine_ready_async();
	void set_engine_ready_callback(tie::EngineReadyResponse response);
	
	bool model_ready_sync();
	void model_ready_async();
	void set_model_ready_callback(tie::ModelReadyResponse response);
	
	// void unary_sync();
	// void unary_async();
	// void result_callback(HelloReply response);

private:
	void grpc_thread_worker(const std::shared_ptr<grpc::CompletionQueue>& cq);
	
	std::unique_ptr<tie::InferenceService::Stub> _stub;

	std::vector<std::thread> _async_grpc_threads;
	std::shared_ptr<grpc::CompletionQueue> _async_completion_queue;

	std::thread _bidi_grpc_thread;
	std::shared_ptr<grpc::CompletionQueue> _bidi_completion_queue;

	AsyncClientBidiCall<tie::InferResponse>* _bidi_call = nullptr;

	// TODO: Enable configuration 
	unsigned num_async_completion_queues = 2u;
	unsigned num_async_threads = 4u;
};

}