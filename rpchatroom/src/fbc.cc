#include <iostream>
#include <memory>
#include <string>
#include <time.h>
#include <thread>


#include <grpc++/grpc++.h>
#include "chat.grpc.pb.h"

using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using chatroom::Request;
using chatroom::Response;
using chatroom::ChatService;
using chatroom::Message;
using chatroom::ListRequest;
using chatroom::SendRequest;
using chatroom::ListResponse;

class ChatClient {

 public:
  ChatClient(std::shared_ptr<Channel> channel)
      : stub_(ChatService::NewStub(channel)) {}
   

  std::string StartChat(const std::string& user) {
        
        ClientContext context;
        std::shared_ptr<ClientReaderWriter<Message, Response> > stream(
        stub_->StartChat(&context));

        std::thread writer([stream,user] {

	    	Message request;
	      	request.set_name(user);
	      	std::string message;
	      	std::getline(std::cin, message);
	      	request.set_message(message);
	      	request.set_timestamp((long int)time(NULL));
	      	stream->Write(request);

        });

        Response reply;
	    while(stream->Read(&reply)) {
	          std::cout << reply.message() << std::endl;
	    }

        writer.join();
        Status status = stream->Finish();
          //     // Act upon its status.
        if (status.ok()) {
          return reply.message();
        } else {
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
          return "RPC failed";
        }

   }
   
   void Join (const std::string& user, const std::string& room) {

     	Request request;
     	request.set_name(user);
     	request.set_room(room);

     	Response reply;

  		ClientContext context;

        Status status = stub_->Join(&context, request, & reply);

        std::cout << reply.message() <<std::endl;
  } 

  void Leave (const std::string& user, const std::string& room) {

     	Request request;
     	request.set_name(user);
     	request.set_room(room);

     	Response reply;

     	ClientContext context;
        Status status = stub_->Leave(&context, request, & reply);
        std::cout << reply.message() <<std::endl;
  }

  void List (const std::string& user) {

  		ListRequest request;
  		request.set_name(user);

  		ListResponse reply;

  	    ClientContext context;
        Status status = stub_->List(&context, request, & reply);
        
        if (status.ok()) {
        	std::cout << "The list of all rooms here: " << std::endl;
        	for (int i = 0; i < reply.allrooms_size(); i++) {
        		std::cout << reply.allrooms(i) <<" ";
        	}
        	std::cout << std::endl;

        	std::cout << "You have joined "<< reply.joinedrooms_size() <<" rooms!" << std::endl;

        	if (reply.joinedrooms_size() != 0) {
	        	for (int i = 0; i < reply.joinedrooms_size(); i++) {
	        		std::cout << reply.joinedrooms(i) <<" ";
	        	}
	        	std::cout << std::endl;
        	}

        } else {

        	std::cout <<"Failed in creation!"<<std::endl;
        }	
  }

  void Send (const std::string& content) {

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


  bool chatmode = false;
  while(true) {


  	  std::string line;
      std::getline(std::cin, line);

	 	 char* dup = (char*)line.c_str();
		 char * token = strtok(dup, " ");

		 if (strcmp(token, "JOIN") == 0) {
		 	std::vector<std::string> commands;
		 	while (token != NULL) {
		 		token =  strtok(NULL, " ");
		 		if (token != NULL)
		 		commands.push_back(std::string(token));
		 	}

		 	if (commands.size() != 1) {
		 		std::cout << "Usage: JOIN ROOM"<< std::endl;
		 		continue;
		 	}

		 	std::string room = commands[0];
		 	chat.Join(user, room);

		 } else if (strcmp(token, "LEAVE") == 0) {

		 	std::vector<std::string> commands;
		 	while (token != NULL) {
		 		token =  strtok(NULL, " ");
		 		if (token != NULL)
		 		commands.push_back(std::string(token));
		 	}

		 	if (commands.size() != 1) {
		 		std::cout << "Usage: LEAVE ROOM"<< std::endl;
		 		continue;
		 	}

		 	std::string room = commands[0];
		 	chat.Leave(user, room);

		 } else if (strcmp(token, "LIST") == 0) {

		 		token =  strtok(NULL, " ");
			 	if (token != NULL) {
			 		std::cout << "Usage: LIST"<< std::endl;
			 		continue;
			 	}
		 		chat.List(user);

		 } else if (strcmp(token, "CHAT") == 0) {

		 		token =  strtok(NULL, " ");
			 	if (token != NULL) {
			 		std::cout << "Usage: CHAT"<< std::endl;
			 		continue;
			 	}
			 	std::cout <<"Now you have switched to chat mode!"<<std::endl;
			 	//looking for better solution
		 		while(true)
  				std::string reply = chat.StartChat(user);


		 } else {

		 		std::cout<<"Usage: JOIN <room>--to join a chat room, LIST\nLEAVE <room> -- to leave a chat room\nCHAT -- to switch to chat mode"<< std::endl;
		 }
 
  }

  return 0;
}