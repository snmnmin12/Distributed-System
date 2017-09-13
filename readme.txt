1. This is the command line program for server, chat room, client socket programming.

2. I used both multi thread and select function to handle multiple chatroom and multiple clients respectively. Util.H and Util.C defines some basic struct and helper function. Network.H and Network.C defines socket programming

3. To run this program, download the files, then run
	make

4. To start server, run ./crsd, to start client run ./crc

5. Client has two modes for connection with master server and chat room server. When started ./crc, the client first connect to master server to do following operations

	Client: CREATE room1
	Server reply: <<Created Port: 53394 and current alive members: 0
	Client: JOIN room1

Now open another terminal run ./crc and type

	Client: JOIN room1

Now the two client can talk with each other in through room1 server. To quit chat, int terminal 2, type
	
	\QUIT

Now the terminal 2 exited the chat mode and go back to master server awaiting further instruction

Now type:

	DELETE room1

all room member in room1 will receive message to show this room will be closed soon.
and the socket for room1 is closed. Client exit chat through '\QUIT' to master server