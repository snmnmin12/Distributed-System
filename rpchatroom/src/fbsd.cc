#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include <grpc++/grpc++.h>
#include "chat.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using chatroom::Request;
using chatroom::Response;
using chatroom::ChatService;
using chatroom::Message;

// Logic and data behind the server's behavior.
class ChatServiceImpl final : public ChatService::Service {
private:
    //map<room,map<user, message>> db;
    std::unordered_map<std::string, std::unordered_map<std::string, Message>> db;
public:
  ChatServiceImpl() {

  }

  Status SayHello(ServerContext* context, const Request* request,
                  Response* reply) override {
    //std::string message;
    std::time_t  t = request->timestamp();
    std::tm * ptm = std::localtime(&t);
    char buffer[32];
    std::strftime(buffer, 32, "%Y-%m-%d-%H:%M:%S", ptm);
    std::cout <<  buffer << ": "<< request->name() << ": " << request->message() << std::endl;
    //std::getline(std::cin, message);
    //reply->set_message(prefix + " " + message+ ": " + request->name());
    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  ChatServiceImpl service;

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}