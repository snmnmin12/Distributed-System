#include <cstdlib>
#include <queue>
#include "fbsd.h"

#define MSG_LIMIT 20
static int server_port = 50051;

Status ChatServiceImpl::CreateConnection (ServerContext* context, const Request* request, Response* reply) {

  	string key = request->name();
  	db[key] = set<string>();
  	//rmsgs[key] = queue<string>();
	pn[key] = ++server_port;
		
	return Status::OK;
} 
	
Status ChatServiceImpl::Register(ServerContext* context, const Request* request, PortNumber* portNumber) {
	string user = request->name();
	int port_num = pn[user];
	
	pn[user] = port_num;     // store the port number for this user in database
	
	portNumber->set_portnum(port_num);
	
	return Status::OK;
}

Status ChatServiceImpl::OldMessages (ServerContext* context, const Request* request, Response* reply) {
	string user = request->name();
	int port_num = pn[user];
	queue<string> msgs;
	
	// read old messages from file
	ifstream file;	
	string filename = user + ".txt";
	file.open(filename, ios::in);
	if(file.is_open()) {
		string tmp;
		while(getline(file, tmp)) {
			msgs.push(tmp);
			if (msgs.size() > MSG_LIMIT)    // only takes limited messages
				msgs.pop();
		}
		file.close();
	}
	
	// send old messages to chatroom
	string address = "localhost:" + to_string(port_num);		
	AntiClient *send = new AntiClient(grpc::CreateChannel(
		address, grpc::InsecureChannelCredentials()));
	while(!msgs.empty()) {  
		string tmp = msgs.front();
		send->SendText(tmp);
		msgs.pop();
	}	
	free(send);

	return Status::OK;
}
  
Status ChatServiceImpl::StartChat(ServerContext* context,  const Message* _request, Response* reply) {

    Message request = *_request;
	string user = request.name();
    //stream->Read(&request);
	
	// format message
	time_t  t = request.timestamp();
    tm * ptm = localtime(&t);
    char buffer[32];
    strftime(buffer, 32, "%Y-%m-%d-%H:%M:%S", ptm);
    cout <<  buffer << ": "<< user << ": " << request.message() << endl;
	
	// store message in the user's own chatroom message_file
	ofstream file;
	string filename = user + ".txt";
	file.open(filename, ios::app);
	file << buffer << ": "<< user << ": " << request.message() << endl;
	file.close();
	file.clear();

	// send message to the user's own chatroom
	int port_num = pn[user];
	string address = "localhost:" + to_string(port_num);		
	AntiClient *send = new AntiClient(grpc::CreateChannel(
		address, grpc::InsecureChannelCredentials()));
	send->SendMsg(request);
	free(send);
	
	// loop through all chatrooms the user has joined
	unordered_map<string, set<string>>::const_iterator got = db.find(user);
	if ( got != db.end() ) {
		for(auto fl : got->second) {
			if ( pn.find(fl) != pn.end() ) {
				port_num = pn[fl];
				address = "localhost:" + to_string(port_num);	
	
				send = new AntiClient(grpc::CreateChannel(
					address, grpc::InsecureChannelCredentials()));
				send->SendMsg(request);
				free(send);
			}
			// write message to the chatroom's file that the user joined
			filename = fl + ".txt";
			file.open(filename, ios::app);
			file << buffer << ": "<< user << ": " << request.message() << endl;
			file.close();
			file.clear();
		}
	}
					
		// reply
		string prefix = "[Server]: ";
		reply->set_message(prefix + std::string(buffer)+request.message());
     
    	return Status::OK;
}

Status ChatServiceImpl::Join (ServerContext* context, const Request* request, Response* reply) {

  	string user = request->name();
  	string room = request->room();
  	string prefix = "[Server]: ";
  	string YOU_ARE_IN = prefix + "YOU ARE ALREADY IN THE ROOM!";
	string NOT_EXIST = prefix + "THE ROOM DOES NOT EXIST!";
  	string SUCCESS = prefix + room + " JOIN SUCCESS!";

  	if (user == room) {   // join user's own room
		reply->set_message(YOU_ARE_IN);
	} else if (db.find(room) != db.end()) {  // join exist room
  		if (db[user].count(room) != 0) {     // already in
  			reply->set_message(YOU_ARE_IN);
  		}
  		else {
  			db[user].insert(room);        // add entry in database
  			reply->set_message(SUCCESS);
  		}
  	} else {               // join not_exist room
  		reply->set_message(NOT_EXIST);
  	}

  	return Status::OK;
} 

Status ChatServiceImpl::Leave(ServerContext* context, const Request* request, Response* reply) {

	string user = request->name();
  	string room = request->room();
  	string prefix = "[Server]: ";
	string CANNOT_LEAVE = prefix + "CANNOT LEAVE YOUR OWN ROOM!";
  	string NOT_EXIST = prefix + "Room NOT EXIST!";
  	string YOU_NOT_IN = prefix + "YOU ARE NOT IN THE ROOM REQUESTED";
  	string SUCCESS = prefix + "LEAVE SUCCESS!";

  	if (user == room) {    // leave user's own room
		reply->set_message(CANNOT_LEAVE);
	} else if (db.find(room) != db.end()) {  // leave exist room
  		if (db[user].count(room) != 0) {  // in the room
  			db[user].erase(room);         // erase entry
  			reply->set_message(SUCCESS);
  		} else {                           // not in the room
  			reply->set_message(YOU_NOT_IN);
  		}
  	} else {                 // leave not_exist room
  		reply->set_message(NOT_EXIST);
  	}

  	return Status::OK;
}

Status ChatServiceImpl::List(ServerContext* context, const ListRequest* request, ListResponse* reply) {
  		
  	string user = request->name();
  	for (auto room : db) {            // get all rooms
  		reply->add_allrooms(room.first);
	}
	reply->add_joinedrooms(user);      // get joined rooms
	unordered_map<string, set<string>>::const_iterator got = db.find(user);
	for (auto room: got->second)   
  		reply->add_joinedrooms(room);

  	return Status::OK;
}

  // Status Send (ServerContext* context, const SendRequest * request, Response* reply) {

  		// return Status::OK;
  // }
  
/* This class is used to connect to the server on client side */
   
string AntiClient::SendMsg(Message& msg) {
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

void AntiClient::SendText(string& text) {
    Response request;
	Response reply;
	request.set_message(text);
	ClientContext context;
    Status status = stub_->SendText(&context, request, &reply);
	if (!status.ok()) {
        cout << status.error_code() << ": " << status.error_message() << endl;
	}
}	

void RunServer() {
  char s[13];
  sprintf(s, "0.0.0.0:%d", server_port);
  string server_address(s,13);
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
	if (argc < 2) {
		cout << "usage: port"<< endl;
		exit(1);
	}
	
	char *endptr;
	server_port = (int)strtol(argv[1], &endptr, 10);
	if (!*argv[1] || *endptr) {
		cerr << "Invalid port number " << argv[1] << endl;
		exit(1);
	}
	if (server_port <= 1024 || server_port >= 65400) {
		cerr << "Invalid port number " << argv[1] << endl;
		exit(1);
	}
	
  RunServer();

  return 0;
}
