#include <iostream>
#include <memory>
#include <string>
#include <time.h>
#include <thread>

#include <pthread.h>
#include <grpc++/grpc++.h>
#include "chat.grpc.pb.h"

#define INIT_PORT 50063

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

static int curr_port = INIT_PORT;

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
  
  void Register(const string& user, int*& port_num) {
	Request request;
	request.set_name(user);
     	PortNumber pn;
	ClientContext context;
        Status status = stub_->Register(&context, request, &pn);
		*port_num = pn.portnum();
	
  }
  
  void OldMessages(const string& user) {
	Request request;
	request.set_name(user);
	Response reply;
	ClientContext context;
	cout << "Recent posts in chatroom " << user << ":" <<endl;
    Status status = stub_->OldMessages(&context, request, &reply);
	
  }

  void  StartChat(const string& user) {
        
        //ClientContext context;
        //shared_ptr<ClientReaderWriter<Message, Response> > stream(
        //stub_->StartChat(&context));
	Response reply;
       while (true) {
		 ClientContext context;
	    	Message request;
	      	request.set_name(user);
	      	string message;
	       	getline(cin, message);
	      	request.set_message(message);
	      	request.set_timestamp((long int)time(NULL));
	      //	stream->Write(request);

        	Status status = stub_->StartChat(&context, request, &reply);
       	//	Response reply;
        	if (!status.ok()) {
          		cout << status.error_code() << ": " << status.error_message() << endl;
        		return;
		}
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
	
	Status SendText (ServerContext* context, const Response* request, Response* reply) override {
		cout << request->message() << endl;
		return Status::OK;
	}
};

void *RunServer(void *port_n) {
  int port_num = *(int*)port_n;   // get the port number
  char s[13];
  sprintf(s, "0.0.0.0:%d", port_num);
  string server_address(s,13);
  AntiServiceImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  unique_ptr<Server> server(builder.BuildAndStart());

  server->Wait();
  free(port_n);
}

int main(int argc, char** argv) {

  if (argc < 4) {
    cout << "usage: hostname port username"<< endl;
    exit(1);
  }
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50052). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  string address = string(argv[1]) + ":" + string(argv[2]);
  //string address = "localhost:50052";
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
				
				int *port_num = (int*)malloc(1);
				// send port number to server so it can connect
				chat.Register(user, port_num);
				pthread_t th;   // new thread to wait for connection from server
				if(pthread_create(&th, NULL, RunServer, (void*)port_num) < 0) {
					cout << "Can't create thread: %s" << endl;
					return 1;
				}	
				cout <<"Port "<< *port_num << " waiting for connection." <<endl;
				cout <<"Now you have switched to chat mode!"<<endl;
				
				chat.OldMessages(user);
				
		 		//while(true)
				chat.StartChat(user);


		 } else {

		 		cout<<"Usage: JOIN <room>--to join a chat room, LIST\nLEAVE <room> -- to leave a chat room\nCHAT -- to switch to chat mode"<< endl;
		 }
 
  }

  return 0;
}
