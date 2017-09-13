/*
Author: Mingmin Song
Date: 09/10/2017
Purpose: Distributed System HW1
*/

#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <string.h>
#include <string>
#include <sstream>
#include <unistd.h>

using namespace std;

//useful static variable to define client id and room ids
static unsigned int c_count = 0;
static int id = 1;
static unsigned int c_room = 100;
static unsigned int p_num = 5000;

#define NETDB_MAX_HOST_NAME_LENGTH 45
#define MAX_CLIENTS 500
#define MAX_ROOM 10
#define MAX_MEMBER 50
#define BUFFER_LENGTH  250


#define SERVER_PORT     3005
// #define BUFFER_LENGTH    250
#define FALSE              0
#define SERVER_NAME     "127.0.0.1"
// #define NETDB_MAX_HOST_NAME_LENGTH 45
/*
Defined useful data structure and functions here!
*/

// data structure to hold room information
typedef struct {
	char room_name[256];
	int port_num;
	int num_members;
	int slave_socket[MAX_MEMBER];
} room;


//data structure to hold client information
//client has file descriptor, id, and each client belongs to one chat room, if not put NULL
typedef struct{
    int fd;
    int id;
    room* proom;
    bool first_time;
    char name[16];
} client;


//this is the help message ,if client type HELP, this message is displayed
extern char help_message[250];

//this is array to hold clients
extern client* clients[MAX_CLIENTS];

//this is the arry to hold room information and 
extern room* room_db[MAX_ROOM];
// extern int room_fd[MAX_ROOM];

//this is the thread to hold room socket, each thread is responsible for one room
extern pthread_t rooms_thread[MAX_ROOM];


// send message to all client connected to this room except it self
void send_message(char*s, client* cli);

// send message to all client connected to this room include itself
void send_all_message(char* s, client* cli);

//This two function manages add connection fd to room database and remove from room database
void add_to_room(room* r, int fd);
void remove_from_room(room* r, int fd);


#endif