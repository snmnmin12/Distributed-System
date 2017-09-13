/*
Author: Mingmin Song
Date: 09/10/2017
Purpose: Distributed System HW1

This is the server socket interface.
Please refer to the Network.H and Network.C for socket connection details.
*/
#include "NetWork.H"
#include "util.H"

//#define SERVER_PORT     3005
//#define FALSE              0

// #define MAX_CLIENTS 100

using namespace std;
// static unsigned int c_count = 0;
// static int id = 1;

int main() {
   MasterConnection master = MasterConnection();
   master.createConnection(NULL, SERVER_PORT);
   return 0;
}
