
#ifndef _FBSD_H_                   // include file only once
#define _FBSD_H_

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <set>

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
using chatroom::PortNumber;
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
		//db<user, rooms> the rooms that user has joined
		unordered_map<string, set<string>> db;
		//pn<user, post_number> post number of that user
		unordered_map<string, int> pn;
		//rmsgs<room, messages> messages posted to that room
		//unordered_map<string, queue<string>> rmsgs;
	//	vector<string> msgs = {"Hello", " I have no idea!", "Third Greeting!"};
		
public:
	ChatServiceImpl()  {}
	/* Constructor */
	Status CreateConnection (ServerContext* context, const Request* request, Response* reply);
	Status Register(ServerContext* context, const Request* request, PortNumber* portNumber);
	Status OldMessages (ServerContext* context, const Request* request, Response* reply);
	Status StartChat(ServerContext* context, const Message* request, Response* reply);
	Status Join (ServerContext* context, const Request* request, Response* reply);
	Status Leave (ServerContext* context, const Request* request, Response* reply);
	Status List (ServerContext* context, const ListRequest* request, ListResponse* reply);
};

// Client class used to send messages to the server on user side
class AntiClient {

private:
  	unique_ptr<AntiService::Stub> stub_;
	
public:
	AntiClient(shared_ptr<Channel> channel)
      : stub_(AntiService::NewStub(channel)) {}
	
	// send messages to user
	string SendMsg(Message& msg);
	// send plain text as string to user
	void SendText(string& text);
};
#endif
