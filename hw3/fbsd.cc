/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*Will Adams and Nicholas Jackson
  CSCE 438 Section 500*/

/* modified by Mingmin Song for cs662*/


#include <ctime>

#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <stdlib.h>
#include <unistd.h>
#include <map>
#include <set>
#include <random>
#include <atomic>
#include <google/protobuf/util/time_util.h>
#include <grpc++/grpc++.h>

#include "fb.grpc.pb.h"
#include "util.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;

using google::protobuf::Timestamp;
using google::protobuf::Duration;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using hw3::Message;
using hw3::ListReply;
using hw3::Request;
using hw3::Reply;
using hw3::MessengerServer;
using hw3::ServerComm;
using hw3::Data;
using hw3::SMessage;

//Client struct that holds a user's username, followers, and users they follow
struct Client {
    std::string username;
    bool connected = true;
    int following_file_size = 0;
    std::vector<std::shared_ptr<Client>> client_followers;
    std::vector<std::shared_ptr<Client>> client_following;
    ServerReaderWriter<Message, Message>* stream = 0;
    bool operator==(const Client& c1) const{
      return (username == c1.username);
    }
};

class ServerClient;
class ServerChatClient;
bool isLeader = false;
bool isMaster = false;
int server_id = 0;
int back_up_id = 1;
std::atomic<bool> startflag{false};

std::shared_ptr<ServerClient> primary = nullptr;


std::string leader_addr = "";
// std::vector<ServerInfo> servers;
std::map<std::string, std::shared_ptr<ServerClient>> sservers;
std::map<std::string, std::shared_ptr<ServerChatClient>> cservers;
//Vector that stores every client that has been created
std::vector<std::shared_ptr<Client>> client_db;
//access the global configuration files for adress of other servers
std::string myname;
std::string myport, machine1, machine2, machine3;
static ParameterReader pd;
std::vector<std::string> ports = pd.getData();

//count the number of heartbeam happend
int count = 0;
//Helper function used to find a Client object given its username
int find_user(std::string username){
      for (int index = 0; index < client_db.size(); index++) {if (client_db[index]->username == username) return index;}
      return -1;
}


class ServerCommImpl final : public ServerComm::Service {

    Status ShakeHand(ServerContext* context, const Reply* request, Reply* reply) {
    	std::string hostname = "0.0.0.0";
    	if (server_id < 4) hostname = machine1;
    	else if (server_id < 7) hostname = machine2;
    	else hostname = machine3;
        if (isLeader) reply->set_msg(std::to_string(1) + "::" + hostname+":"+ports[server_id]);
        else reply->set_msg(std::to_string(0) + "::" + hostname+":"+ports[server_id]);
        return Status::OK;
    }

    Status ID(ServerContext* context, const Reply* request, Reply* reply) {
        reply->set_msg(std::to_string(server_id));
        return Status::OK;
    }

    Status DataSync(ServerContext* context, const Reply* request, Data* data) {

        if (isLeader) {
            for (auto cli : client_db) {
                Data::ClientData* clidata = data->add_clidata();
                clidata->set_username(cli->username);
                for (auto& p : cli->client_followers) {  clidata->add_followers(p->username);}
                for (auto& p : cli->client_following) {  clidata->add_following(p->username);}
            }
        }
        return Status::OK;
    }
};

class ServerClient{
public:
    ServerClient(std::shared_ptr<Channel> channel)
      : stub_(ServerComm::NewStub(channel)) {}

    bool ShakeHand(const std::string& address) { 
        Reply request, reply;
        request.set_msg(address);
        ClientContext context;
        Status status = stub_->ShakeHand(&context, request, &reply);
        std::string temp = reply.msg();
        if (temp.substr(0, temp.find("::")).compare("1") == 0){
            temp.erase(0, temp.find("::") + 2);
            leader_addr = temp;
        }
        if (status.ok()) {
            // sleep(1);
            if(address.size() != 0 && count%10 == 0) {
             std::cout << "Shakehand server id: " << server_id << " --> " << address << std::endl;
            }
            return true;
        }
        return false;
    }

    int ID() {
        Reply request, reply;
        request.set_msg("Hi");
        ClientContext context;
        Status status = stub_->ID(&context, request, &reply);
        if (status.ok()) return atoi(reply.msg().c_str());
        return 1000000;
    }

    void DataSync() {
        Reply request;
        Data data;
        request.set_msg("Hi");
        ClientContext context;
        Status status = stub_->DataSync(&context, request, &data);

        for (int i = 0; i < data.clidata_size(); i++) {

            Data::ClientData clidata = data.clidata(i);
            std::string username = clidata.username();

            if (find_user(username) == -1) {
                auto c = std::make_shared<Client>(); c->username = username; client_db.push_back(c);
                int index = find_user(username);
                int size1 = clidata.followers().size();
                int size2 = clidata.following().size();
                for (int i = 0; i < size1; i++) {
                  std::string username1 = clidata.followers(i);
                  int j = 0;
                  for (;j < client_db.size(); j++) {
                        if (client_db[j]->username == username1)  break;
                  }
                  if (j == client_db.size()) {
                      std::shared_ptr<Client> client = std::make_shared<Client>();
                      client->username = username1;
                      client_db.push_back(client);
                  }
                 client_db[index]->client_followers.push_back(client_db[j]);
              }

              for (int i = 0; i < size2; i++) {
                  std::string username1 = clidata.following(i);
                  int j = 0;
                  for (;j < client_db.size(); j++) {
                        if (client_db[j]->username == username1)  break;
                  }
                  if (j == client_db.size()) {
                      auto client = std::make_shared<Client>();
                      client->username = username1;
                      client_db.push_back(client);
                  }
                 client_db[index]->client_following.push_back(client_db[j]);
              }
          }
      }
  }

  private:
          std::unique_ptr<ServerComm::Stub> stub_;
};

class ServerChatClient {
private:
  //std::unique_ptr<ServerToServer::Stub> stub_;
  std::unique_ptr<MessengerServer::Stub> stub_;
public:
  ServerChatClient(std::shared_ptr<Channel> channel)
      : stub_(MessengerServer::NewStub(channel)) {}

  bool ShakeHand(const std::string& key) {
        Reply request, reply;
        request.set_msg(key);
        ClientContext context;
        Status status = stub_->ShakeHand(&context, request, &reply);
        if (status.ok()) return true;
        return false;
  }

  void Login(const std::string username){
        Request request;
        request.set_username("$"+username);
        Reply reply;
        ClientContext context;
        Status status = stub_->Login(&context, request, &reply);
        if(!status.ok()) { std::cout << status.error_code() << ": " << status.error_message()<< std::endl;}
  }

  void Join(const std::string username1, const std::string username2){
        Request request;
        request.set_username("$"+username1);
        request.add_arguments(username2);
        Reply reply;
        ClientContext context;
        Status status = stub_->Join(&context, request, &reply);
        if (!status.ok()) { std::cout << status.error_code() << ": " << status.error_message()<< std::endl;}
  }

  void Leave(const std::string username1, const std::string username2){
        Request request;
        request.set_username("$"+username1);
        request.add_arguments(username2);
        Reply reply;
        ClientContext context;
        Status status = stub_->Leave(&context, request, &reply);
        if (!status.ok()) { std::cout << status.error_code() << ": " << status.error_message()<< std::endl;}
  }

  void Forward(std::string from, std::string username, const Message& message) {
        Reply reply;
        ClientContext context;
        SMessage smsg;
        Message* mess = new Message(message);
        smsg.set_from(from);
        smsg.set_username(username);
        smsg.set_allocated_msg(mess);
        stub_->Forward(&context, smsg, &reply);
  }

};


// Logic and data behind the server's behavior.
class MessengerServiceImpl final : public MessengerServer::Service {

  //communicate with server at initialization time
  Status ServerCommunicate(ServerContext* context, const Data* data, Reply* reply) {
          // std::string username = data->username();
          // if (find_user(username) == -1) {
          //       Client c; c.username = username; client_db.push_back(c);
          // }
          // //first find the username of the client
          // int index = find_user(username);
          // int size1 = data->followers().size();
          // int size2 = data->following().size();

          // //second find the user name of the followers
          // for (int i = 0; i < size1; i++) {
          //     std::string username1 = data->followers(i);
          //     int j = 0;
          //     for (;j < client_db.size(); j++) {
          //           if (client_db[j].username == username1)  break;
          //     }
          //     if (j == client_db.size()) {
          //         Client client;
          //         client.username = username1;
          //         client_db.push_back(client);
          //     }
          //    client_db[index].client_followers.push_back(&client_db[j]);
          // }

          // for (int i = 0; i < size2; i++) {
          //     std::string username1 = data->following(i);
          //     int j = 0;
          //     for (;j < client_db.size(); j++) {
          //           if (client_db[j].username == username1)  break;
          //     }
          //     if (j == client_db.size()) {
          //         Client client;
          //         client.username = username1;
          //         client_db.push_back(client);
          //     }
          //    client_db[index].client_following.push_back(&client_db[j]);
          // }

          return Status::OK;
  }

  Status ShakeHand(ServerContext* context, const Reply* request, Reply* reply) {
      return Status::OK;
  }
  
  //Sends the list of total rooms and joined rooms to the client
  Status List(ServerContext* context, const Request* request, ListReply* list_reply) override {
      auto user = client_db[find_user(request->username())];
      int index = 0;
      for(const auto& c : client_db){  list_reply->add_all_rooms(c->username); }
      for(const auto& p : user->client_following) { list_reply->add_joined_rooms(p->username); }
      for(const auto& p : user->client_followers) { list_reply->add_follower_rooms(p->username);}
      return Status::OK;
  }

  //Sets user1 as following user2
  Status Join(ServerContext* context, const Request* request, Reply* reply) override {
    // if(servers.size() > 0) { for(auto& p : servers) }
    std::string username1 = request->username();
    std::string username2 = request->arguments(0);
    if (cservers.size() > 0 && username1[0] != '$') { for(auto& p: cservers) {p.second->Join(username1, username2);}}
    if (username1[0] == '$') username1 = username1.substr(1, username1.size()-1);

    int join_index = find_user(username2);
    //If you try to join a non-existent client or yourself, send failure message
    if(join_index < 0 || username1 == username2)
      reply->set_msg("Join Failed -- Invalid Username");
    else{
      auto user1 = client_db[find_user(username1)];
      auto user2 = client_db[join_index];
      //If user1 is following user2, send failure message
      if(std::find(user1->client_following.begin(), user1->client_following.end(), user2) != user1->client_following.end()){
	           reply->set_msg("Join Failed -- Already Following User");
        return Status::OK;
      }
      // std::cout << myname <<": "<< user1->username <<"->" << user2->username <<std::endl;
      user1->client_following.push_back(user2);
      user2->client_followers.push_back(user1);
      reply->set_msg("Join Successful");
    }
    return Status::OK; 
  }

  //Sets user1 as no longer following user2
  Status Leave(ServerContext* context, const Request* request, Reply* reply) override {
    std::string username1 = request->username();
    std::string username2 = request->arguments(0);
    if (cservers.size() > 0 && username1[0] != '$') { for(auto& p: cservers) {p.second->Leave(username1, username2);}}
    if (username1[0] == '$') username1 = username1.substr(1, username1.size()-1);

    int leave_index = find_user(username2);
    //If you try to leave a non-existent client or yourself, send failure message
    if(leave_index < 0 || username1 == username2)
      reply->set_msg("Leave Failed -- Invalid Username");
    else{
      auto user1 = client_db[find_user(username1)];
      auto user2 = client_db[leave_index];
      //If user1 isn't following user2, send failure message
      if(std::find(user1->client_following.begin(), user1->client_following.end(), user2) == user1->client_following.end()){
	          reply->set_msg("Leave Failed -- Not Following User");
            return Status::OK;
      }
      // find the user2 in user1 following and remove
      user1->client_following.erase(find(user1->client_following.begin(), user1->client_following.end(), user2)); 
      // find the user1 in user2 followers and remove
      user2->client_followers.erase(find(user2->client_followers.begin(), user2->client_followers.end(), user1));
      reply->set_msg("Leave Successful");
    }
    return Status::OK;
  }

  //Called when the client startd and checks whether their username is taken or not
  Status Login(ServerContext* context, const Request* request, Reply* reply) override {

    if (isMaster) {
       std::string address="";
       std::string port = "";
       int flag = rand()%2;
       if (!flag) {
             std::map<std::string, std::shared_ptr<ServerChatClient>>::reverse_iterator it;
             for (it = cservers.rbegin(); it != cservers.rend(); ++it) 
             {       
                     if(it->second->ShakeHand(it->first)) {
                        address =  it->first; break;
                     }
             }
       } else {
             std::map<std::string, std::shared_ptr<ServerChatClient>>::iterator it;
             for (it = cservers.begin(); it != cservers.end(); ++it) 
             {       
                     if(it->second->ShakeHand(it->first)) {
                        address =  it->first; break;
                     }
             }
       }
       // }
       std::cout << "redirecting to " << address << std::endl;
       reply->set_msg("$"+address);
       return Status::OK;
    }
    // std::set<std::string> usernames;
    std::string username = request->username();
    if(cservers.size()>0 && username[0] != '$') { for(const auto& p: cservers) { p.second->Login(username);}}

    if (username[0] == '$') username = username.substr(1, username.size()-1);
    // usernames.insert(request->username());
    // for (const auto& username : usernames) {
        int user_index = find_user(username);
        if(user_index < 0){
                std::shared_ptr<Client> c = std::make_shared<Client>();
                c->username = username;
                client_db.push_back(c);
                reply->set_msg("Login Successful!");
        }
        else{ 
               auto user = client_db[user_index];
               //if(user->connected)
                 //   reply->set_msg("Invalid Username");
               //else
               {
                std::string msg = "Welcome Back " + user->username;
    	          reply->set_msg(msg);
                user->connected = true;
              }
        }
  // }
  return Status::OK;
  }

  Status Chat(ServerContext* context, ServerReaderWriter<Message, Message>* stream) override {
    Message message;
    std::shared_ptr<Client> c;
    //Read messages until the client disconnects
    while(stream->Read(&message)) {
      std::string username = message.username();
      int user_index = find_user(username);
      c = client_db[user_index];
      //Write the current message to "username.txt"
      std::string filename = username+".txt";
      google::protobuf::Timestamp temptime = message.timestamp();
      std::string time = google::protobuf::util::TimeUtil::ToString(temptime);
      std::string fileinput = time+" :: "+message.username()+":"+message.msg()+"\n";
      //"Set Stream" is the default message from the client to initialize the stream
      if(message.msg() != "Set Stream") {
          if (c->stream == 0) c->stream = stream;
          if(isLeader) 
          {
                std::ofstream user_file(filename,std::ios::app|std::ios::out|std::ios::in);
                user_file << fileinput;
          } 
      }
      //If message = "Set Stream", print the first 20 chats from the people you follow
      else{
        if(c->stream==0) c->stream = stream;
        std::string line;
        std::vector<std::string> newest_twenty;
        std::ifstream in(username+"following.txt");
        int count = 0;
        //Read the last up-to-20 lines (newest 20 messages) from userfollowing.txt
        while(getline(in, line)){
             if(c->following_file_size > 20){
	           if(count < c->following_file_size-20){ count++; continue;}
             }
             newest_twenty.push_back(line);
        }
        Message new_msg; 
 	      //Send the newest messages to the client to be displayed
        for (auto& msg : newest_twenty) {new_msg.set_msg(msg); stream->Write(new_msg);}    
        continue;
      }
      //Send the message to each follower's stream
      // std::cout <<"follower size: "<<c->client_followers.size() << std::endl;
      for(const auto& s: cservers) {s.second->Forward(username, username, message);}
      for(auto& p : c->client_followers){
        // std::cout <<"Ready to forward"<<std::endl;
        if(p->stream!=0 && p->connected)   p->stream->Write(message);
        else if (p->stream == 0) { if(cservers.size() > 0) {for(const auto& s: cservers) {s.second->Forward("", p->username, message);}}}
        // std::cout <<"Forward completed!"<<std::endl;
        //For each of the current user's followers, put the message in their following.txt file
        if(isLeader) 
        {
          std::string temp_username = p->username;
          std::string temp_file = temp_username + "following.txt";
          std::ofstream following_file(temp_file,std::ios::app|std::ios::out|std::ios::in);
          following_file << fileinput;
          p->following_file_size++;
          std::ofstream user_file(temp_username + ".txt",std::ios::app|std::ios::out|std::ios::in);
          user_file << fileinput;
        }
      }


    }
    //If the client disconnected from Chat Mode, set connected to false
    c->connected = false;
    return Status::OK;
  }

  Status Forward(ServerContext* context,  const SMessage* smessage, Reply * reply) override {
      std::string from = smessage->from();
      std::string username = smessage->username();
      const Message& message = smessage->msg();
      // std::cout <<"User name: "<< username << std::endl;
      int index = find_user(username);
      // std::cout <<"Index: "<< index << std::endl;
      if (index != -1 && from !=username) {
      if (client_db[index]->stream != 0 && client_db[index]->connected)  {
            // std::cout << "useranme: " << client_db[index].username << std::endl;
            client_db[index]->stream->Write(message);
        }
      }
      if (from.size() !=0 && isLeader) {
          int client_index = find_user(from);
          auto c = client_db[client_index];
          std::string filename = from+".txt";
          google::protobuf::Timestamp temptime = message.timestamp();
          std::string time = google::protobuf::util::TimeUtil::ToString(temptime);
          std::string fileinput = time+" :: "+message.username()+":"+message.msg()+"\n";
          std::ofstream user_file(filename,std::ios::app|std::ios::out|std::ios::in);
          user_file << fileinput;

          for(const auto& p : c->client_followers){
            std::string temp_username = p->username;
            std::string temp_file = temp_username + "following.txt";
            std::ofstream following_file(temp_file,std::ios::app|std::ios::out|std::ios::in);
            following_file << fileinput;
            p->following_file_size++;
            std::ofstream user_file(temp_username + ".txt",std::ios::app|std::ios::out|std::ios::in);
            user_file << fileinput;
          }
      }

      return Status::OK;
  }

};

//initialize the servers
void initlizedServers() {
    
    for (int i = 2; i <  ports.size(); i++) {
        if (i == server_id) continue;
        std::string hostname = machine1;
        if (i >= 4 && i <=6) hostname = machine2;
        else if (i>6) hostname = machine3; 
        std::string address = hostname+":"+ports[i];
        std::shared_ptr<ServerClient> messenger = std::make_shared<ServerClient>(grpc::CreateChannel(
        address, grpc::InsecureChannelCredentials())); 
        std::shared_ptr<ServerChatClient> messenger2 = std::make_shared<ServerChatClient>(grpc::CreateChannel(
        address, grpc::InsecureChannelCredentials())); 
        messenger->DataSync();
        sservers[address] = messenger;
        cservers[address] = messenger2;
        // }
    }
    if (isMaster) {
        std::string address = "0.0.0.0:"+ports[0];
        primary = std::make_shared<ServerClient>(grpc::CreateChannel(address, grpc::InsecureChannelCredentials())); 
    }
}

// void* RunCommServer(void* invalid) {
//   // std::string server_address = "localhost:"+port_no;
//   std::string server_address = hostname +":"+myport;
//   ServerCommImpl service;

//   ServerBuilder builder;
//   // Listen on the given address without any authentication mechanism.
//   builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
//   // Register "service" as the instance through which we'll communicate with
//   // clients. In this case it corresponds to an *synchronous* service.
//   builder.RegisterService(&service);
//   // Finally assemble the server.
//   std::unique_ptr<Server> server(builder.BuildAndStart());
//   std::cout << "Server listening on " << server_address << std::endl;
//   // Wait for the server to shutdown. Note that some other thread must be
//   // responsible for shutting down the server for this call to ever return.
//   server->Wait();
// }

void RunServer(std::string hostname) {
  std::string server_address = "0.0.0.0:"+myport;
  MessengerServiceImpl service;
  ServerCommImpl service2;

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  builder.RegisterService(&service2);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

bool election() {
     int i = 0;
     int category = server_id/3;
    for (auto& item : sservers) {
      if (i < 3*category || i >=3*(category+1) ) continue;
      i++;
      int id = item.second->ID();
      if (server_id < id) {
        isLeader = true;
      } else {
        isLeader = false;
      }
    }
    if (isLeader) {
      std::cout << server_id  << " Is Leader." << std::endl;
    }
    return isLeader;
}

void* restart(void* data) {
  std::string* addres = reinterpret_cast<std::string*>(data);
  int index = (*addres).find(":");
  if (index == -1) return 0;
  std::string pot = (*addres).substr(index+1, (*addres).size()-index-1);
  int index2 = pd.getServiceID(pot);
  std::string cmd = "./fbsd";
  cmd = cmd + " -z " + machine3; 
  cmd = cmd + " -y " + machine2; 
  cmd = cmd + " -x " + machine1;
  cmd = cmd + " -i " + std::to_string(index2);
  if (index2 == 2 || index2 == 4 || index2 == 7) cmd = cmd + " -l";
  if(system(cmd.c_str()) == -1){
    std::cerr << "Error: Could not start new process" << '\n';
  }
  return 0;

}

void* heartBeatMonitor(void* invalid) {
    pthread_t startNewServer_tid = -1;
    while(true) {
      sleep(1);
      if (isMaster && !isLeader) {
                if (primary != nullptr && !primary->ShakeHand("")) {
                    std::string address = "0.0.0.0:"+ports[0];
                    primary = std::make_shared<ServerClient>(grpc::CreateChannel(address, grpc::InsecureChannelCredentials())); 
                    if (!primary->ShakeHand("")) {isLeader = true;}
                    else isLeader = false;
                } else { isLeader = false;}
      }

      if (isMaster && isLeader) {
	    
            if(!startflag) {std::cout<<"Servers are initializing, please wait........"<<std::endl;}
            for (auto& item : sservers) {
                auto& key = item.first;
                int index = key.find(":");
                std::string pot = key.substr(index+1, key.size()-index-1);
                std::string* addr = new std::string(key);
                auto& conn = item.second;
                bool pulse = conn->ShakeHand(key); 
                if (!pulse) {
                    {
                          std::cout<<"Have lost the connection!"<<std::endl;
                          if (pd.getServiceID(pot) < 4) { //service is local
                          		pthread_create(&startNewServer_tid, NULL, &restart, (void*)addr);
                          }
                          std::string address = key;
                          std::shared_ptr<ServerClient> messenger = std::make_shared<ServerClient>(grpc::CreateChannel(
                          address, grpc::InsecureChannelCredentials())); 
                         if (messenger->ShakeHand(myname)) { sservers[key] = messenger;}
                     }
                }
                sleep(1);
            }
	    count += 1;
            startflag = true;
      } 
    }
}

int main(int argc, char** argv) {
  
  // std::string port = "3010";
  // hostname = "localhost";
  myport = "3123";
  machine1 = "0.0.0.0";
  int opt = 0;
  while ((opt = getopt(argc, argv, "x:y:z:mli:")) != -1){
    switch(opt) {
      case 'x':
          machine1 = optarg; break;
      case 'y':
      	  machine2 = optarg; break;
      case 'z':
      	  machine3 = optarg; break;
      case 'l':
          isLeader = true; break;
      case 'm':
          isMaster = true; break;
      case 'i':
          server_id = atoi(optarg); break;
      default:
	  std::cerr << "Invalid Command Line Argument\n";
    }
  }
  srand(time(0));
  myport = ports[server_id];
  pthread_t client_tid = -1, heartbeat_tid = -1;
  initlizedServers();
  sleep(1);
  pthread_create(&heartbeat_tid, NULL, &heartBeatMonitor, (void*) NULL);  
  RunServer(machine1);
  return 0;
}
