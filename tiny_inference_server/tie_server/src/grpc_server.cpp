#include "grpc_server.hpp"

#include <grpcpp/completion_queue.h>
#include <grpcpp/server_builder.h>
#include <spdlog/spdlog.h>

#include "rpc/rpc_pool.hpp"
#include "rpc/rpc_model_infer.hpp"
#include "rpc/rpc_model_list.hpp"
#include "rpc/rpc_model_load.hpp"
#include "rpc/rpc_model_metadata.hpp"
#include "rpc/rpc_model_ready.hpp"
#include "rpc/rpc_model_unload.hpp"

#include "rpc/rpc_server_live.hpp"
#include "rpc/rpc_server_metadata.hpp"
#include "rpc/rpc_server_ready.hpp"


namespace tie::server
{
grpc_server::grpc_server(const std::shared_ptr<engine::engine_interface>& engine)
    : _engine{ engine }
    , _is_running{ false }
{
    spdlog::debug("create grpc_server");
    rpc_pool::get();
}

grpc_server::~grpc_server()
{
    spdlog::debug("delete grpc_server");
    stop();
}

bool grpc_server::start(const server_config& config)
{
    if (_is_running)
        return false;

    spdlog::debug("start grpc_server");

    _service = std::make_shared<inference::GRPCInferenceService::AsyncService>();

    const auto max_msg_payload_byte = 4'000'000; // 4 Megabytes
    const auto server_uri = config.grpc_address + ":" + config.grpc_port;
    const auto num_inference_threads = config.grpc_inference_threads;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_uri, grpc::InsecureServerCredentials());
    // builder.SetMaxReceiveMessageSize(max_msg_payload_byte);
    // builder.SetMaxSendMessageSize(max_msg_payload_byte);
    // builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 1000);
    // builder.SetDefaultCompressionLevel(GRPC_COMPRESS_LEVEL_HIGH);
    // builder.SetDefaultCompressionAlgorithm(GRPC_COMPRESS_GZIP);
    builder.RegisterService(_service.get());

    for (auto queue_index = 0; queue_index < num_inference_threads; ++queue_index)
    {
        std::shared_ptr<grpc::ServerCompletionQueue> queue = builder.AddCompletionQueue();
        _rpc_inference_queues.emplace_back(queue);
    }

    _rpc_common_queue = builder.AddCompletionQueue(/*false*/);
    _server = builder.BuildAndStart();
    _is_running = _server != nullptr;

    // Inferece RPCs (one RPC per thread)
    for (const auto& queue : _rpc_inference_queues)
    {
        _server_threads.emplace_back(
            [this](const auto& queue) 
            {
                rpc_pool::get().create_rpc<rpc_model_infer>(_service, queue, _engine)->execute();
                rpc_handler(queue); 
            },
            queue);
    }

    // Common RPCs (all RPC in a single thread)
    _server_threads.emplace_back(
        [this](const auto& queue) 
        {
            rpc_pool::get().create_rpc<rpc_server_live>(_service, queue, _engine)->execute();
            rpc_pool::get().create_rpc<rpc_server_ready>(_service, queue, _engine)->execute();
            rpc_pool::get().create_rpc<rpc_server_metadata>(_service, queue, _engine)->execute();
            rpc_pool::get().create_rpc<rpc_model_list>(_service, queue, _engine)->execute();
            rpc_pool::get().create_rpc<rpc_model_load>(_service, queue, _engine)->execute();
            rpc_pool::get().create_rpc<rpc_model_metadata>(_service, queue, _engine)->execute();
            rpc_pool::get().create_rpc<rpc_model_ready>(_service, queue, _engine)->execute();
            rpc_pool::get().create_rpc<rpc_model_unload>(_service, queue, _engine)->execute();
            rpc_handler(queue); 
        }, 
        _rpc_common_queue);

    if(_is_running)
    {
        spdlog::info("gRPC Server running at: {}", server_uri);
    }
    else
    {
        spdlog::error("Error starting gRPC Server at: {}", server_uri);
    }

    return _is_running;
}

void grpc_server::stop()
{
    if (!_is_running)
        return;

    spdlog::info("stop grpc_server");
    rpc_pool::get().release();

    _server->Shutdown();

    auto drain_completion_queue = [](const auto& queue)
    {
        void* tag = nullptr;
        bool ok = false;
        while (queue->Next(&tag, &ok))
        {
            // do nothing
        };
    };

    _rpc_common_queue->Shutdown();
    drain_completion_queue(_rpc_common_queue);

    for (auto&& queue : _rpc_inference_queues)
    {
        queue->Shutdown();
        drain_completion_queue(queue);
    }

    for (auto&& t : _server_threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    _is_running = false;
}

void grpc_server::rpc_handler(const std::shared_ptr<grpc::ServerCompletionQueue>& cq)
{
    void* tag = nullptr;
    bool ok = false;
    
    while (cq->Next(&tag, &ok))
    {
        auto rpc = static_cast<tie::server::rpc*>(tag);
        if(!ok)
        {
            rpc_pool::get().remove_rpc(rpc->id);
            continue;
        }

        if(!rpc->execute())
        {
            rpc_pool::get().remove_rpc(rpc->id);
        }        
    }

    spdlog::trace("rpc_handler shutdown");
}

}