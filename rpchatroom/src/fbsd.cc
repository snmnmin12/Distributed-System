#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <set>
#include <queue>

#include <grpc++/grpc++.h>
#include "chat.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using grpc::ServerReaderWriter;

using chatroom::Request;
using chatroom::Response;
using chatroom::ChatService;
using chatroom::Message;
using chatroom::ListRequest;
using chatroom::SendRequest;
using chatroom::ListResponse;


// Logic and data behind the server's behavior.
class ChatServiceImpl final : public ChatService::Service {

private:
	//db<room, user>
		std::unordered_map<std::string, std::set<std::string>> db;
		std::unordered_map<std::string, std::queue<std::string>> rmsgs;
		std::vector<std::string> msgs = {"Hello", " I have no idea!", "Third Greeting!"};

public:
  ChatServiceImpl() { }
 Status CreateConnection (ServerContext* context, const Request* request, Response* reply) {

  		std::string key = request->name();
  		db[key] = std::set<std::string>();
  		rmsgs[key] = std::queue<std::string>();
		
		return Status::OK;
  } 

  Status StartChat(ServerContext* context, ServerReaderWriter<Response, Message>* stream) override {

    		Message request;
    		(stream->Read(&request)); {

    		std::string message;
      		std::time_t  t = request.timestamp();
      		std::tm * ptm = std::localtime(&t);
      		char buffer[32];
      		std::strftime(buffer, 32, "%Y-%m-%d-%H:%M:%S", ptm);
      		std::cout <<  buffer << ": "<< request.name() << ": " << request.message() << std::endl;
     		 //reply
      		Response reply;
      		std::string prefix = "[Server]: ";
      		reply.set_message(prefix + msgs[msgs.size()-1]);
      		stream->Write(reply);
       		msgs.push_back(request.message());
    }
    return Status::OK;
  }

  Status Join (ServerContext* context, const Request* request, Response* reply) {

  		std::string user = request->name();
  		std::string room = request->room();
  		std::string prefix = "[Server]: ";
  		std::string YOU_ARE_IN = prefix + "YOU ARE ALREADY IN THE ROOM!";
  		std::string SUCCESS = prefix + room + " JOIN SUCCESS!";

  		if (db.find(room) != db.end()) {
  				if (db[room].count(user) != 0) {
  					reply->set_message(YOU_ARE_IN);
  				}
  				else {
  					db[room].insert(user);
  					reply->set_message(SUCCESS);
  				}
  		} else {

  			db[room].insert(user);
  			reply->set_message(SUCCESS);
  		}

  		return Status::OK;
  } 

  Status Leave (ServerContext* context, const Request* request, Response* reply) {

  		std::string user = request->name();
  		std::string room = request->room();
  		std::string prefix = "[Server]: ";
  		std::string NOT_EXIST = prefix + "Room NOT EXIST!";
  		std::string YOU_NOT_IN = prefix + "YOU ARE NOT IN THE ROOM REQUESTED";
  		std::string SUCCESS = prefix + "LEAVE SUCCESS!";

  		if (db.find(room) != db.end()) {
  			if (db[room].count(user) != 0) {
  				db[room].erase(user);
  				reply->set_message(SUCCESS);
  			} else {

  				reply->set_message(YOU_NOT_IN);
  			}
  		} else {
  			reply->set_message(NOT_EXIST);
  		}

  	 	return Status::OK;
  }

  Status List (ServerContext* context, const ListRequest* request, ListResponse* reply) {
  		
  		std::string user = request->name();
		reply->add_joinedrooms(user);
  		for (auto room : db) {
  			reply->add_allrooms(room.first);
  			if (room.second.count(user) != 0)
  				reply->add_joinedrooms(room.first);
  		}

  		return Status::OK;
  }

  Status Send (ServerContext* context, const SendRequest * request, Response* reply) {

  		return Status::OK;
  }
  
  // Status SendAll (SendAllRequest) {

  // 	returns Response;
  // } 

  // Status RecvAll (RecvAllRequest) {

  // 	returns ChatResponse;
  // }


};

void RunServer() {
  std::string server_address("0.0.0.0:50052");
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
