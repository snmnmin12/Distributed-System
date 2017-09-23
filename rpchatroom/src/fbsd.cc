#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <set>
#include <queue>

#include <grpc++/grpc++.h>
#include "chat.grpc.pb.h"

using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ClientContext;
using grpc::ServerWriter;
using grpc::ClientReader;
using grpc::ServerReaderWriter;
using grpc::ClientReaderWriter;
using grpc::Status;
using grpc::Channel;

using chatroom::ChatService;
using chatroom::AntiService;
using chatroom::Request;
using chatroom::Response;
using chatroom::Message;
using chatroom::ListRequest;
using chatroom::SendRequest;
using chatroom::ListResponse;

class AntiClient;

// Logic and data behind the server's behavior.
class ChatServiceImpl final : public ChatService::Service {

private:
	//db<room, user>
		unordered_map<string, set<string>> db;
		unordered_map<string, queue<string>> rmsgs;
		vector<string> msgs = {"Hello", " I have no idea!", "Third Greeting!"};

public:
  ChatServiceImpl() { }
 Status CreateConnection (ServerContext* context, const Request* request, Response* reply) {

  		string key = request->name();
  		db[key] = set<string>();
  		rmsgs[key] = queue<string>();
		
		return Status::OK;
  } 

  Status StartChat(ServerContext* context, ServerReaderWriter<Response, Message>* stream) override {

    		Message request;
    		(stream->Read(&request)); {

    		string message;
      		time_t  t = request.timestamp();
      		tm * ptm = localtime(&t);
      		char buffer[32];
      		strftime(buffer, 32, "%Y-%m-%d-%H:%M:%S", ptm);
      		cout <<  buffer << ": "<< request.name() << ": " << request.message() << endl;
     		 //reply
      		Response reply;
      		string prefix = "[Server]: ";
      		reply.set_message(prefix + msgs[msgs.size()-1]);
      		stream->Write(reply);
       		msgs.push_back(request.message());
    }
    return Status::OK;
  }

  Status Join (ServerContext* context, const Request* request, Response* reply) {

  		string user = request->name();
  		string room = request->room();
  		string prefix = "[Server]: ";
  		string YOU_ARE_IN = prefix + "YOU ARE ALREADY IN THE ROOM!";
  		string SUCCESS = prefix + room + " JOIN SUCCESS!";

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

  		string user = request->name();
  		string room = request->room();
  		string prefix = "[Server]: ";
  		string NOT_EXIST = prefix + "Room NOT EXIST!";
  		string YOU_NOT_IN = prefix + "YOU ARE NOT IN THE ROOM REQUESTED";
  		string SUCCESS = prefix + "LEAVE SUCCESS!";

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
  		
  		string user = request->name();
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

/* This class is used to connect to the server on client side */
class AntiClient {

private:
  	unique_ptr<AntiService::Stub> stub_;
	
public:
	AntiClient(shared_ptr<Channel> channel)
      : stub_(AntiService::NewStub(channel)) {}
   
	string SendMsg(Message& msg) {
     	Response reply;
		ClientContext context;
        Status status = stub_->SendMsg(&context, msg, &reply);
		if (status.ok()) {
			return reply.message();
		} else {
			cout << status.error_code() << ": " << status.error_message()
                << endl;
		return "RPC failed";
		}
	}	
};

void RunServer() {
  string server_address("0.0.0.0:50052");
  ChatServiceImpl service;

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  unique_ptr<Server> server(builder.BuildAndStart());
  cout << "Server listening on " << server_address << endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}
