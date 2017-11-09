CSCE 662 HW3 Submission

Compile the code using the provided makefile:

    make

To clear the directory (and remove .txt files):
   
    make clean

To run the server on port 3010 (the default port used for testing):

    ./fbsd  -p 3010

To run the client on the localhost, on port 3010, and with username "user1": 

    ./fbc -h localhost -p 3010 -u user1

 The three scripts files are used for start the processes on servers. We have one master server with master node, worker node and two other servers with only worker node, so client can connect to the master node and master node will assign connection to client. When processes crashes, the other process will help to restart the program.

