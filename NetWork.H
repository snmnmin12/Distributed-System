/*
Author: Mingmin Song
Date: 09/10/2017
Purpose: Distributed System HW1

Master socket class is responsible to hande client CREATE, JOIN ,DELETE operations
Chatroom socket class is responsible to receive message from client and forward message to other members in the same chat room
Client socket class is responsible to connect to clients
*/

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include "util.H"


//request the thread to create a chatroom, requested by server to handle create command;
void *request_thread(void* args);


//handle JOIN, CREATE and DELETE room command and act accordingly
void handleJoin(const char* roomname, client* cli);
void handleCreate(const char* roomname, client* cli);
void handleDelete(const char* roomname, client* cli, fd_set& master);


//This function to keep receiving and responding message from client, parsing input on server side and act accordingly.
void master_handle(client* cli, fd_set& master);

//This function to keep receiving and responding from clients, parsing input on chat room side and act accordingly.
void room_handle(client* cli, fd_set& roomset);


// Class for Master Server collecltion
class MasterConnection {
private:
	int fd;
public:
	MasterConnection();
	int getfd() {return fd;}
	//create socket connection, bind, listen and accept
	void createConnection(char * host, int portNo, int backlog=10);
	~MasterConnection() {close(fd);}
};

//class to handle room collection
class RoomConnection {
private:
	int fd;
public:
	bool open;
	RoomConnection() {open = true; fd = 0;}
	int getfd() {return fd;}
	//create socket connection, bind, listen and accept
	void createConnection(const char * host, room* r, int backlog = 10);
	// void connectToClient();
};

//class to handle client connection
class ClientConnection {
private:
	int fd;
	char server[45];
public:
	volatile bool open;
	ClientConnection(){	fd = 0; open = true;};
	int getfd() {return fd;}
	//first connection to server socket with portNo received from server
	void firstConnection(char* host, int portNo);
	//second, create connection to connect to room socket with portNo received from server
	void createConnection(char * host, int portNo);
	~ClientConnection() {close(fd);};
};

#endif