
#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>
#include <services.grpc.pb.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using inference::GRPCInferenceService;
using inference::ServerLiveRequest;
using inference::ServerLiveResponse;

class GreeterClient
{
public:
    // Constructor
    GreeterClient(std::shared_ptr<Channel> channel)
        : stub_(GRPCInferenceService::NewStub(channel))
    {
    }

    // Assembles the client's payload, sends it and presents the response back from the server.
    bool ServerLive()
    {
        // Data we are sending to the server.
        ServerLiveRequest request;

        // Container for the data we expect from the server.
        ServerLiveResponse reply;

        // Context for the client.
        // It could be used to convey extra information to the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_->ServerLive(&context, request, &reply);

        // Act upon its status.
        if (status.ok())
        {
            return reply.live();
        }
        else
        {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return false;
        }
    }

private:
    std::unique_ptr<GRPCInferenceService::Stub> stub_;
};

void Run()
{
    // Instantiate the client. It requires a channel, out of which the actual RPCs are created.
    // This channel models a connection to an endpoint (in this case, localhost at port 50051).
    // We indicate that the channel isn't authenticated (use of InsecureChannelCredentials()).
    GreeterClient greeter(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
    bool reply = greeter.ServerLive();
    
    std::cout << reply << std::endl;
}

int main()
{
    Run();
    return 0;
}