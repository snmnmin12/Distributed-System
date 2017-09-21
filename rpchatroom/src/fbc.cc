#include <iostream>
#include <memory>
#include <string>
#include <time.h>

#include <grpc++/grpc++.h>
#include "chat.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using chatroom::Request;
using chatroom::Response;
using chatroom::ChatService;

class ChatClient {
 public:
  ChatClient(std::shared_ptr<Channel> channel)
      : stub_(ChatService::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string SayHello(const std::string& user) {
    // Data we are sending to the server.
      Request request;
      request.set_name(user);
      std::string message;
      std::getline(std::cin, message);
      request.set_message(message);
      request.set_timestamp((long int)time(NULL));

      // Container for the data we expect from the server.
      Response reply;

      // Context for the client. It could be used to convey extra information to
      // the server and/or tweak certain RPC behaviors.
      ClientContext context;

      // The actual RPC.
      Status status = stub_->SayHello(&context, request, &reply);

      // Act upon its status.
      if (status.ok()) {
        return reply.message();
      } else {
        std::cout << status.error_code() << ": " << status.error_message()
                  << std::endl;
        return "RPC failed";
      }
  }

 private:
  std::unique_ptr<ChatService::Stub> stub_;
};

int main(int argc, char** argv) {

  if (argc < 4) {
    std::cout << "usage: hostname port username"<< std::endl;
    exit(1);
  }
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  std::string address = std::string(argv[1]) + ":" + std::string(argv[2]);
  //std::string address = "localhost:50051";
  ChatClient chat(grpc::CreateChannel(
      address, grpc::InsecureChannelCredentials()));

  std::string user(argv[3]);
  while (1) {
  		std::string reply = chat.SayHello(user);
  }

  return 0;
}