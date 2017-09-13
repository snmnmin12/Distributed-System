/*
Author: Mingmin Song
Date: 09/10/2017
Purpose: Distributed System HW1
*/
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <string.h>
#include <string>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "NetWork.H"
#include "util.H"
using namespace std;

char help_message[250] = "CREATE roomname--to create a chat room\nJOIN roomname--to join a chat room\nDELETE roomname--to delete a chat room\nOr HELP--to get help\n";

client* clients[MAX_CLIENTS];
// ClientConnection* client_connections[MAX_CLIENTS];
room* room_db[MAX_ROOM];
int room_fd[MAX_ROOM];

pthread_t rooms_thread[MAX_ROOM];


void *request_thread(void* args) {
	room* r = (room*)(args);
	RoomConnection r_conn = RoomConnection();
	r_conn.createConnection(NULL,r);
	return NULL;
}


void handleJoin(const char* roomname, client* cli) {

	int i = 0;
	for (; i < MAX_ROOM; i++) {
		if (room_db[i] && strcmp(room_db[i]->room_name, roomname) == 0) {
			break;
		}
	}
	char buffer[BUFFER_LENGTH];

	if (i == MAX_ROOM) {
		sprintf(buffer, "<<Your roomname does not exist: %s\n", roomname);
	} else {
		while(room_db[i]->port_num == 0);
		sprintf(buffer, "<<Created Port: %d and current alive members: %d\n", room_db[i]->port_num, room_db[i]->num_members);
		//printf("%s", buffer);
	}
	write(cli->fd, buffer, BUFFER_LENGTH);
}

void handleCreate(const char* roomname, client* cli){
	int i = 0;
	for (; i < MAX_ROOM; i++) {
		if (room_db[i] && strcmp(room_db[i]->room_name, roomname) == 0) {
			break;
		}
	}
	char buffer[BUFFER_LENGTH];
	if (i < MAX_ROOM) {
		sprintf(buffer, "<<This roomname already exists: %s\n", roomname);
	} else {
		//create new room and save it in room database
		room* r = new room();
		strcpy(r->room_name, roomname);
		r->port_num = 0;
		r->num_members = 0;
		memset(r->slave_socket, 0, MAX_MEMBER);
		int j = 0;
		for (; j < MAX_ROOM; j++) {
			if (!room_db[j]) break;
		}
		room_db[j] = r;
		// RoomConnection* r_conn = new RoomConnection();
		// r_conn.createConnection(NULL,r);
		//pthread_create(&rooms_thread[j], NULL, request_thread, (void*)(r_conn));
		pthread_create(&rooms_thread[j], NULL, request_thread, (void*)(r));
		sprintf(buffer, "<<Room Created with roomname: %s, you can join at any time!\n", roomname);
	}
	write(cli->fd, buffer, BUFFER_LENGTH);

}

void handleDelete(const char* roomname, client* cli, fd_set& master){
	int i = 0;
	for (; i < MAX_ROOM; i++) {
		if (room_db[i] && strcmp(room_db[i]->room_name, roomname) == 0) {
			break;
		}
	}
	char buffer[BUFFER_LENGTH];
	if (i  == MAX_ROOM) {
		sprintf(buffer, "<<This roomname does not exists: %s\n", roomname);
		write(cli->fd, buffer, BUFFER_LENGTH);
	} else {
		for (int j = 0; j < MAX_MEMBER; j++) {
			int fd =  room_db[i]->slave_socket[j];
			if (fd == 0) continue;
			sprintf(buffer, "<<This room is going to be closed and please quit!\r\n");
			write(fd, buffer, sizeof(buffer));
			memset(buffer, 0, BUFFER_LENGTH);
			close(fd);
		}

		for (int j = 0; j < MAX_CLIENTS; j++) {
	      if (clients[j] && clients[j]->proom == room_db[i]) {
	          clients[j]->proom = NULL;
	          FD_CLR(clients[j]->fd, &master);
	          delete clients[j];
	          clients[j] = NULL;
	        }
	    }


		delete room_db[i];
		pthread_detach(rooms_thread[i]);
    	pthread_cancel(rooms_thread[i]);
		room_db[i] = NULL;
		close(room_fd[i]);
		room_fd[i] = 0;

		sprintf(buffer, "<<Room has been deleted!\n");
		write(cli->fd, buffer, BUFFER_LENGTH);
	}
}

void ClientConnection::firstConnection(char* host, int portNo) {

		struct sockaddr_in addr;
		struct hostent *hostp;

		memset(&addr, 0, sizeof(addr));

		int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
		//test error sd < 0
		memset(&addr, 0, sizeof(addr));
		addr.sin_family      = AF_INET;
		addr.sin_port        = htons(portNo);
		addr.sin_addr.s_addr = inet_addr(host);


		if (addr.sin_addr.s_addr == (unsigned long)INADDR_NONE)   {
		 hostp = gethostbyname(host);
		 if (hostp == (struct hostent *)NULL) {
		    printf("Host not found --> ");
		    //break;
		 }
		 memcpy(&addr.sin_addr,
		        hostp->h_addr,
		        sizeof(addr.sin_addr));
		}

		if (connect(listen_fd, (struct sockaddr *)&addr, sizeof(addr))) {
			printf("Can not find \n");
			return;
		}

	  //handle connect
	  char buffer[BUFFER_LENGTH];
	  memset(buffer, 0, sizeof(buffer));

	  printf("=> Awaiting confirmation from the server...\n"); //line 40
      recv(listen_fd, buffer, BUFFER_LENGTH, 0);
      printf("%s", buffer);
      // printf("=> Connection confirmed, you are good to go...\n");
      bool isExit = false;
      int chat_port_num = -1;

      while(!isExit) {
      		fd_set readset;
      		FD_ZERO(&readset);
	        FD_SET(STDIN_FILENO, &readset);
	        FD_SET(listen_fd, &readset);

		    if (select(listen_fd+1, &readset, NULL, NULL, NULL) < 0) {
	            perror("select");
	            return;
	        }

	         if (FD_ISSET(STDIN_FILENO, &readset)) {
	                int received = read(STDIN_FILENO, buffer, BUFFER_LENGTH);
	                buffer[received-1] = 0;
	                //fgets(buffer,BUFFER_LENGTH, stdin);
	                send(listen_fd, buffer, BUFFER_LENGTH, 0);
	         } else {
	                int received = read(listen_fd, buffer, BUFFER_LENGTH);
	                buffer[received] = 0;
	                printf("%s", buffer);
	                char *pch = buffer;
	                //get port number from reply
		            pch = strtok(buffer," ");
		            if (strcmp(pch, "<<Created") == 0) {
			            pch = strtok (NULL," ");
			            pch = strtok (NULL," ");
			            chat_port_num = atoi(pch);
			            isExit = true;
		        	} else if (strcmp(pch, "QUIT") == 0) {
		        		isExit = true;
		        	}

	         }
      }
      close(listen_fd);
      c_count--;
      if (chat_port_num > 0 ) {
      		printf("Now it is going to join chat room!\n");
      		createConnection(host, chat_port_num);
      		printf("Come back again to the master room!\n");
      		firstConnection(host, portNo);
      }


}
void ClientConnection::createConnection(char * host, int portNo) {
		
		struct sockaddr_in addr;
		struct hostent *hostp;

		memset(&addr, 0, sizeof(addr));

		int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
		//test error sd < 0
		memset(&addr, 0, sizeof(addr));
		addr.sin_family      = AF_INET;
		addr.sin_port        = htons(portNo);
		addr.sin_addr.s_addr = inet_addr(host);


		if (addr.sin_addr.s_addr == (unsigned long)INADDR_NONE)   {
		 hostp = gethostbyname(host);
		 if (hostp == (struct hostent *)NULL) {
		    printf("Host not found --> ");
		    //break;
		 }
		 memcpy(&addr.sin_addr,
		        hostp->h_addr,
		        sizeof(addr.sin_addr));
		}

		if (connect(listen_fd, (struct sockaddr *)&addr, sizeof(addr))) {
			printf("Can not find in second connection \n");
			return;
		}

	  fd = listen_fd;

	  //handle connect
	  char buffer[BUFFER_LENGTH];
	  memset(buffer, 0, sizeof(buffer));

	  printf("=>Awaiting confirmation from...\n"); //line 40
      recv(listen_fd, buffer, BUFFER_LENGTH, 0);
      printf("%s", buffer);
      // printf("=> Connection confirmed, you are good to go...\n");
      while(true) {
         fd_set readset;
         // int readable = 0;
         FD_ZERO(&readset);
         FD_SET(STDIN_FILENO, &readset);
         FD_SET(listen_fd, &readset);
         if (select(listen_fd+1, &readset, NULL, NULL, NULL) < 0) {
            perror("select");
            return;
         }

         if (FD_ISSET(STDIN_FILENO, &readset)) {
                int received = read(STDIN_FILENO, buffer, BUFFER_LENGTH);
                buffer[received-1] = 0;
                if (buffer[0] == '\\'){
                	char *command;
                	command = strtok(buffer," ");
                	if (!strcmp(command,"\\QUIT")) {
                		//send(listen_fd, buffer, BUFFER_LENGTH, 0);
                		break;
                	}
                }
                //fgets(buffer,BUFFER_LENGTH, stdin);
                send(listen_fd, buffer, BUFFER_LENGTH, 0);
         } else {
                int received = read(listen_fd, buffer, BUFFER_LENGTH);
                buffer[received] = 0;
                printf("%s", buffer);
         }

      }
      close(fd);
      printf("Connection is closed\n");

}

void master_handle(client* cli, fd_set& master) {

	if (cli->proom != NULL) return;

    char buffer1[BUFFER_LENGTH];
    char buffer2[BUFFER_LENGTH];

    int rlen = 0;
    if ((rlen = read(cli->fd, buffer1, BUFFER_LENGTH)) > 0) {
          buffer1[rlen] = '\0';

          string s = buffer1;
          char *pch = NULL;
          pch = strtok(buffer1," ");

          if (pch == NULL) {
	          sprintf(buffer2,"<<Nothing Received\r\n");
	          write(cli->fd, buffer2, sizeof(buffer2));

          } else if(!strcmp(pch,"JOIN")) {

          	 pch  = strtok(NULL," "); 
          	 if (pch == NULL) {
          	 	sprintf(buffer2,"<<Please provide roomname!\r\n");
          	 	write(cli->fd, buffer2, sizeof(buffer2));
          	 } else {
          	 	printf("<<Join Request Received from %s for room: %s\n", cli->name, pch);
          	 	handleJoin(pch, cli);
          	 }

          } else if(!strcmp(pch,"CREATE")) {

          	 pch  = strtok(NULL," "); 
          	 if (pch == NULL) {
          	 	sprintf(buffer2,"<<Please provide roomname!\r\n");
          	 	write(cli->fd, buffer2, sizeof(buffer2));
          	 } else {
	          	 printf("<<Create Request Received from %s for room: %s\n", cli->name, pch);
	          	 handleCreate(pch, cli);
          	}

          } else if (!strcmp(pch,"DELETE")) {

          	 pch  = strtok(NULL," "); 
          	 if (pch == NULL) {
          	 	sprintf(buffer2,"<<Please provide roomname!\r\n");
          	 	write(cli->fd, buffer2, sizeof(buffer2));
          	 } else {
	          	 printf("<<Delete Request received from %s for room: %s\n", cli->name, pch);
	          	 handleDelete(pch, cli,master);
	          }

          } else if (!strcmp(pch, "HELP")) {

          		write(cli->fd, help_message, sizeof(help_message));
          } else {

	          	sprintf(buffer2,"<<Not understand %s\r\n", buffer1);
	          	write(cli->fd, buffer2, sizeof(buffer2));
          }

    } else {

	    close(cli->fd);
	    FD_CLR(cli->fd, &master);

	    for (int i = 0; i < MAX_CLIENTS; i++) {
	      if (clients[i] == cli) {
	          clients[i] = NULL;
	          break;
	        }
	    }
	    //send message in the server
	    printf("<<Disconnected from server: %s\n", cli->name);
	    delete cli;
	    cli = NULL;
	    c_count--;
	}
}

MasterConnection::MasterConnection(){
	fd = 0; 
	memset(room_db, 0, sizeof(room_db));
	memset(clients, 0, sizeof(clients));
}

void MasterConnection::createConnection(char * host, int portNo, int backlog) {
   	
   	   struct sockaddr_in addr;
	   memset(&addr, 0, sizeof(addr));
	   addr.sin_family      = AF_INET;
	   addr.sin_port        = htons(portNo);
	   addr.sin_addr.s_addr = htonl(INADDR_ANY);

	   int sd = socket(AF_INET, SOCK_STREAM, 0);
	   int rc = bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	   // test error rc < 0
	   if (rc < 0) {
	      printf("bind failed!\n");
	      return;
	   }

	   rc = listen(sd, backlog);
	   // test error rc< 0
	   if (rc < 0) {
	      printf("listen failed!\n");
	      return;
	   }

	   socklen_t len = sizeof(addr);
	   if (getsockname(sd, (struct sockaddr *)&addr, &len) == -1) {
		    perror("getsockname");
		    return;
		}

	   //printf("port no is: %d\n",addr.sin_port);

	   fd = sd;

	   int max_fd  = 0;
	   fd_set master;
	   int serSize = sizeof(addr);
	   int sd2 = 0;
	   char buffer[BUFFER_LENGTH];
	   
	   while(1) {

	   		FD_ZERO(&master);
			FD_SET(fd, &master);
			max_fd = fd;

			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (clients[i]) {
					FD_SET(clients[i]->fd, &master);
					if (clients[i]->fd > max_fd)
						max_fd = clients[i]->fd ;
				}
			}

	   		int sock = select(max_fd+1, &master, NULL, NULL, NULL);
	   		if (sock < 0)  {
	   			printf("Error in selection\r\n");
	   		}
   			if (FD_ISSET(fd, &master)) {

   				int sd2 = accept(fd,(struct sockaddr*)&addr, (socklen_t*)&serSize);

   				client* cli = new client();
   				cli->fd = sd2;
			    cli->id = id++;
			    cli->proom = NULL;

			    sprintf(cli->name, "Guest%d", cli->id);
			    add_to_clients(cli);

		    	memset(buffer,0, sizeof(buffer));
			    printf("<<Connected to server.\n");
			    sprintf(buffer, "<<Welcome to the chat room server, %s, TYPE HELP for help\r\n", cli->name);
			    write(cli->fd, buffer, BUFFER_LENGTH);
			    c_count += 1;

			} else {

				for (int i = 0; i < MAX_CLIENTS; i++) {
					if (clients[i] && FD_ISSET(clients[i]->fd, &master)) {
						 master_handle(clients[i], master);
					}
				}
			}
	   }
	   printf("Connection is closed\n");	
}


void room_handle(client* cli, fd_set& roomset) {

    char buffer1[BUFFER_LENGTH];
    char buffer2[BUFFER_LENGTH];

    memset(buffer1,0,sizeof(buffer1));
    memset(buffer2,0,sizeof(buffer2));

    int rlen = 0;

    if ((rlen = read(cli->fd, buffer1, BUFFER_LENGTH)) > 0) {
        buffer1[rlen] = '\0';
        string s = buffer1;
	    if (strlen(buffer1)) {
	          memset(buffer2,0,sizeof(buffer2));
	          sprintf(buffer2,"[%s] %s\n", cli->name, buffer1);
	          send_message(buffer2, cli);
      	}
    } else {

	    close(cli->fd);
	    FD_CLR(cli->fd, &roomset);

	    sprintf(buffer2, "%s has left this room!\r\n", cli->name);
	    send_message(buffer2, cli);

	   	remove_from_room(cli->proom, cli->fd);
	    remove_from_clients(cli);

	    c_count--;
	}
}


void RoomConnection::createConnection(const char * host, room* r, int backlog) {
   		
   	   int portNo = r->port_num;
   	   struct sockaddr_in addr;
	   memset(&addr, 0, sizeof(addr));
	   addr.sin_family      = AF_INET;
	   addr.sin_port        = htons(portNo);
	   addr.sin_addr.s_addr = htonl(INADDR_ANY);

	   int sd = socket(AF_INET, SOCK_STREAM, 0);
	   int rc = bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	   // test error rc < 0
	   if (rc < 0) {
	      printf("bind failed!\n");
	      return;
	   }

	   socklen_t len = sizeof(addr);
	   if (getsockname(sd, (struct sockaddr *)&addr, &len) == -1) {
		    perror("getsockname");
		    return;
		}

	    // get room index in data base and save the room fd;
			for (int i = 0; i < MAX_ROOM; i++) {
				if (room_db[i] && strcmp(room_db[i]->room_name, r->room_name) == 0) {
					room_fd[i] = sd;
					break;
				}
			}

	    //

	   r->port_num = ntohs(addr.sin_port);

	   char buffer[BUFFER_LENGTH];

	   rc = listen(sd, backlog);
	   // test error rc< 0
	   if (rc < 0) {
	      printf("listen failed!\n");
	      return;
	   }
	   fd = sd;

	   //select and poll
	   fd_set roomset;
	   int max_fd = 0;
	   int serSize = sizeof(addr);
	   // int cli = 0;

	   while(open) {

	   		FD_ZERO(&roomset);
			FD_SET(fd, &roomset);
			max_fd = fd;

			for (int i = 0; i < MAX_MEMBER; i++) {
				if (r->slave_socket[i] != 0) {
					int sd2 = r->slave_socket[i];
					FD_SET(sd2, &roomset);
					if (sd2 > max_fd)
						max_fd = sd2;
				}
			}

	   		int sock = select(max_fd+1, &roomset, NULL, NULL, NULL);
	   		if (sock < 0)  {
	   			printf("Error in selection\n");
	   		}
   			if (FD_ISSET(fd, &roomset)) {

   				int sd2 = accept(fd,(struct sockaddr*)&addr, (socklen_t*)&serSize);
   				if (max_fd < sd2) max_fd  = sd2;

   				client* cli = new client();
   				cli->fd = sd2;
			    cli->id = id++;
			    cli->proom = r;
			    sprintf(cli->name, "Guest%d", cli->id);
			    add_to_clients(cli);

		    	add_to_room(r, cli->fd);
		    	memset(buffer,0, sizeof(buffer));
			    sprintf(buffer, "<<Welcome to room %s: %s and TYPE \\QUIT to exit!\r\n", r->room_name,cli->name);
			    send_all_message(buffer, cli);
			    c_count += 1;
			} else {

				for (int i = 0; i < MAX_CLIENTS; i++) {
					if (clients[i] && FD_ISSET(clients[i]->fd, &roomset)) {
						 room_handle(clients[i], roomset);
					}
				}
			}

	   }
	   printf("You have exited the chat room\n");	
}