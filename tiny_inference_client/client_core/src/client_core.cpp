#include <client_core.hpp>
#include "grpc_client.hpp"
#include <memory>

namespace tie::client_core
{
    
client_core::client_core(const std::string &channel_address) : _impl{ std::make_unique<tie::client_core::grpc_client>(channel_address) }
{
}

client_core::~client_core()
{
}

bool client_core::engine_ready_sync()
{
    return _impl->engine_ready_sync();
}

void client_core::engine_ready_async()
{
    _impl->engine_ready_async();
}

void client_core::set_engine_ready_callback(const std::function<void(bool)>& callback)
{
    _impl->set_engine_ready_callback(std::forward<decltype(callback)>(callback));
}

bool client_core::model_ready_sync()
{
    return _impl->model_ready_sync();
}

void client_core::model_ready_async()
{
    _impl->model_ready_async();
}

void client_core::set_model_ready_callback(const std::function<void(bool)>& callback)
{
    _impl->set_model_ready_callback(std::forward<decltype(callback)>(callback));
}



/*

void client_core::start_infer_stream()
{
	if (_bidi_call && _bidi_call->call_state != AsyncClientBidiCall<tie::InferResponse>::CallState::FINISH)
	{
		std::cout << "Warning: bidi call must be in write state before call read." << std::endl;;
		return;
	}

	_bidi_call = new AsyncClientBidiCall<tie::InferResponse>();
	_bidi_call->call_state = AsyncClientBidiCall<tie::InferResponse>::CallState::CREATE;
	// _bidi_call->set_response_callback([this](tie::InferResponse&& response){ result_callback(response); });
	// _bidi_call->rpc = _stub->PrepareAsyncInferStream(&_bidi_call->context, _bidi_completion_queue.get());
	_bidi_call->rpc->StartCall((void *)_bidi_call);
}

void client_core::stop_infer_stream()
{
	_bidi_call->call_state = AsyncClientBidiCall<tie::InferResponse>::CallState::DONE;
	_bidi_call->rpc->WritesDone((void *)_bidi_call);
}

void client_core::send_infer_stream_request(const std::string &msg, bool is_last)
{
	if (_bidi_call->call_state != AsyncClientBidiCall<tie::InferResponse>::CallState::CREATE
	&& _bidi_call->call_state != AsyncClientBidiCall<tie::InferResponse>::CallState::WRITE)
	{
		std::cout << "Warning: bidi call must be in CREATE or WRITE state before call WRITE." << std::endl;;
		return;
	}

	_bidi_call->call_state = AsyncClientBidiCall<tie::InferResponse>::CallState::WRITE;
	tie::InferRequest request;
	// request.set_data(data);
	// request.set_model_name(model_name);
	// request.set_shape(shape);
	
	// TODO: check grpc::WriteOptions()
	_bidi_call->rpc->Write(request, (void *)_bidi_call);
}

void client_core::read_infer_stream_response()
{
	if (_bidi_call->call_state != AsyncClientBidiCall<tie::InferResponse>::CallState::WRITE)
	{
		std::cout << "Warning: BIDI call must be in write state before call read." << std::endl;;
		return;
	}
	
	_bidi_call->call_state = AsyncClientBidiCall<tie::InferResponse>::CallState::READ;
	_bidi_call->rpc->Read(&_bidi_call->response, (void *)_bidi_call);
}

*/


}
