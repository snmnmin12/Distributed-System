/*
Author: Mingmin Song
Date: 09/10/2017
Purpose: Distributed System HW1

This is the client socket interface.
Please refer to the Network.H and Network.C for socket connection details.
*/

#include "NetWork.H"
#include "util.H"

int main(int argc, char *argv[]) {
    char server[NETDB_MAX_HOST_NAME_LENGTH];
    unsigned int  port_num;
    if (argc > 1)  strcpy(server, argv[1]);
    else  strcpy(server, SERVER_NAME);

    if (argc > 2)  port_num = atoi(argv[2]);
    else port_num = SERVER_PORT;

    ClientConnection clien = ClientConnection();
    clien.firstConnection(server, port_num);
    //clien.createConnection(server, SERVER_PORT);
   // int    listen_fd =0, rc, bytesReceived;
   // char   buffer[BUFFER_LENGTH];
   // char   server[NETDB_MAX_HOST_NAME_LENGTH];
   // struct sockaddr_in serveraddr;
   // struct hostent *hostp;

   //    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
   //    //test error sd < 0

   //    if (argc > 1)  strcpy(server, argv[1]);
   //    else  strcpy(server, SERVER_NAME);

   //    memset(&serveraddr, 0, sizeof(serveraddr));
   //    serveraddr.sin_family      = AF_INET;
   //    serveraddr.sin_port        = htons(SERVER_PORT);
   //    serveraddr.sin_addr.s_addr = inet_addr(server);


   //    if (serveraddr.sin_addr.s_addr == (unsigned long)INADDR_NONE)      {
   //       hostp = gethostbyname(server);
   //       if (hostp == (struct hostent *)NULL) {
   //          printf("Host not found --> ");
   //          //break;
   //       }
   //       memcpy(&serveraddr.sin_addr,
   //              hostp->h_addr,
   //              sizeof(serveraddr.sin_addr));
   //    }

   //    rc = connect(listen_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
   //    // test error rc < 0

   //    printf("=> Awaiting confirmation from the server...\n"); //line 40
   //    recv(listen_fd, buffer, BUFFER_LENGTH, 0);
   //    printf("%s", buffer);
   //    // printf("=> Connection confirmed, you are good to go...\n");

   //    while(1) {
   //       fd_set readset;
   //       int readable = 0;
   //       FD_ZERO(&readset);
   //       FD_SET(STDIN_FILENO, &readset);
   //       FD_SET(listen_fd, &readset);
   //       if (select(listen_fd+1, &readset, NULL, NULL, NULL) < 0) {
   //          perror("select");
   //          return -1;
   //       }

   //       if (FD_ISSET(STDIN_FILENO, &readset)) {
   //              int received = read(STDIN_FILENO, buffer, BUFFER_LENGTH);
   //              buffer[received-1] = 0;
   //              //fgets(buffer,BUFFER_LENGTH, stdin);
   //              send(listen_fd, buffer, BUFFER_LENGTH, 0);
   //       } else {
   //              int received = read(listen_fd, buffer, BUFFER_LENGTH);
   //              buffer[received] = 0;
   //              printf("%s", buffer);

   //       }

   //    }

   //  close(listen_fd);
   return 0;
}
