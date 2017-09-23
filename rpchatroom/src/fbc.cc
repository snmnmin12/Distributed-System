#include <iostream>
#include <memory>
#include <string>
#include <time.h>
#include <thread>


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

class AntiServiceImpl;

class ChatClient {

private:
  	unique_ptr<ChatService::Stub> stub_;
	
public:
  ChatClient(shared_ptr<Channel> channel)
      : stub_(ChatService::NewStub(channel)) {}
   
  void CreateConnection(const string& user) {

	Request request;
    request.set_name(user);

    Response reply;
  	ClientContext context;
    Status status = stub_->CreateConnection(&context, request, &reply);

  }

  string StartChat(const string& user) {
        
        ClientContext context;
        shared_ptr<ClientReaderWriter<Message, Response> > stream(
        stub_->StartChat(&context));

        thread writer([stream,user] {

	    	Message request;
	      	request.set_name(user);
	      	string message;
	      	getline(cin, message);
	      	request.set_message(message);
	      	request.set_timestamp((long int)time(NULL));
	      	stream->Write(request);

        });

        Response reply;
	    while(stream->Read(&reply)) {
	          cout << reply.message() << endl;
	    }

        writer.join();
        Status status = stream->Finish();
          //     // Act upon its status.
        if (status.ok()) {
          return reply.message();
        } else {
          cout << status.error_code() << ": " << status.error_message()
                    << endl;
          return "RPC failed";
        }

   }
   
   void Join (const string& user, const string& room) {

     	Request request;
     	request.set_name(user);
     	request.set_room(room);

     	Response reply;

  	ClientContext context;

        Status status = stub_->Join(&context, request, & reply);

        cout << reply.message() <<endl;
  } 

  void Leave (const string& user, const string& room) {

     	Request request;
     	request.set_name(user);
     	request.set_room(room);

     	Response reply;

     	ClientContext context;
        Status status = stub_->Leave(&context, request, & reply);
        cout << reply.message() <<endl;
  }

  void List (const string& user) {

  	ListRequest request;
  	request.set_name(user);

  	ListResponse reply;

  	ClientContext context;
        Status status = stub_->List(&context, request, & reply);
        
        if (status.ok()) {
        	cout << "The list of all rooms here: " << endl;
        	for (int i = 0; i < reply.allrooms_size(); i++) {
        		cout << reply.allrooms(i) <<" ";
        	}
        	cout << endl;

        	cout << "You have joined "<< reply.joinedrooms_size() <<" rooms!" << endl;

        	if (reply.joinedrooms_size() != 0) {
	        	for (int i = 0; i < reply.joinedrooms_size(); i++) {
	        		cout << reply.joinedrooms(i) <<" ";
	        	}
	        	cout << endl;
        	}

        } else {

        	cout <<"Failed in creation!"<<endl;
        }	
  }

  void Send (const string& content) {

  }
};

/* The server on the client side to receive message from chatroom server */
class AntiServiceImpl final : public AntiService::Service {
	
public:
	AntiServiceImpl() { }
	Status SendMsg (ServerContext* context, const Message* msg, Response* reply) override {
		time_t  t = msg->timestamp();
		tm * ptm = localtime(&t);
		char buffer[32];
  		strftime(buffer, 32, "%Y-%m-%d-%H:%M:%S", ptm);
      	cout <<  buffer << ": "<< msg->name() << ": " << msg->message() << endl;
		reply->set_message("Message received.");
		
		return Status::OK;
	} 
};

int main(int argc, char** argv) {

  if (argc < 4) {
    cout << "usage: hostname port username"<< endl;
    exit(1);
  }
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  string address = string(argv[1]) + ":" + string(argv[2]);
  //string address = "localhost:50051";
  ChatClient chat(grpc::CreateChannel(
      address, grpc::InsecureChannelCredentials()));

  string user(argv[3]);
  chat.CreateConnection(user);

  bool chatmode = false;
  while(true) {

      string line;
      getline(cin, line);

	 	 char* dup = (char*)line.c_str();
		 char * token = strtok(dup, " ");

		 if (strcmp(token, "JOIN") == 0) {
		 	vector<string> commands;
		 	while (token != NULL) {
		 		token =  strtok(NULL, " ");
		 		if (token != NULL)
		 		commands.push_back(string(token));
		 	}

		 	if (commands.size() != 1) {
		 		cout << "Usage: JOIN ROOM"<< endl;
		 		continue;
		 	}

		 	string room = commands[0];
		 	chat.Join(user, room);

		 } else if (strcmp(token, "LEAVE") == 0) {

		 	vector<string> commands;
		 	while (token != NULL) {
		 		token =  strtok(NULL, " ");
		 		if (token != NULL)
		 		commands.push_back(string(token));
		 	}

		 	if (commands.size() != 1) {
		 		cout << "Usage: LEAVE ROOM"<< endl;
		 		continue;
		 	}

		 	string room = commands[0];
		 	chat.Leave(user, room);

		 } else if (strcmp(token, "LIST") == 0) {

		 		token =  strtok(NULL, " ");
			 	if (token != NULL) {
			 		cout << "Usage: LIST"<< endl;
			 		continue;
			 	}
		 		chat.List(user);

		 } else if (strcmp(token, "CHAT") == 0) {

		 		token =  strtok(NULL, " ");
			 	if (token != NULL) {
			 		cout << "Usage: CHAT"<< endl;
			 		continue;
			 	}
			 	cout <<"Now you have switched to chat mode!"<<endl;
			 	//looking for better solution
		 		while(true)
  				string reply = chat.StartChat(user);


		 } else {

		 		cout<<"Usage: JOIN <room>--to join a chat room, LIST\nLEAVE <room> -- to leave a chat room\nCHAT -- to switch to chat mode"<< endl;
		 }
 
  }

  return 0;
}
