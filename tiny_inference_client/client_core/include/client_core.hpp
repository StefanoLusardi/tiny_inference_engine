#pragma once

#include <memory>
#include <string>
#include <functional>

#if defined(_WIN32) || defined(_WIN64)
	#define API_EXPORT_TIE_CLIENT __declspec(dllexport)
#else
	#define API_EXPORT_TIE_CLIENT __attribute__ ((visibility ("default")))
#endif

namespace tie::client_core
{
class grpc_client;
class API_EXPORT_TIE_CLIENT client_core final
{
public:
	explicit client_core(const std::string& channel_address);
	~client_core();

	void start_infer_stream();
	void stop_infer_stream();
	void send_infer_stream_request(const std::string& msg, bool is_last = false);
	void read_infer_stream_response();

	bool engine_ready_sync();
	void engine_ready_async();
	void engine_ready_async(const std::function<void(bool)>& callback);
	void set_engine_ready_callback(const std::function<void(bool)>& callback);

	bool model_ready_sync();
	void model_ready_async();
	void set_model_ready_callback(const std::function<void(bool)>& callback);

private:	
	std::unique_ptr<tie::client_core::grpc_client> _impl;
};

}