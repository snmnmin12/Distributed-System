#include "util.H"

void send_message(char*s, client* cli) {

	  room* r = cli->proom;
	  for (int i = 0; i < r->num_members; i++) {
	  		if (r->slave_socket[i] !=0 && r->slave_socket[i] != cli->fd) 
	  		{
	  			write(r->slave_socket[i], s, strlen(s));
	  		}
	  }
}

void send_all_message(char* s, client* cli) {

	  room* r = cli->proom;
	  for (int i = 0; i < r->num_members; i++) {
	  		if (r->slave_socket[i] !=0) 
	  		{
	  			write(r->slave_socket[i], s, strlen(s));
	  		}
	  }
}

void add_to_room(room* r, int fd) {
	if (r->num_members + 1 == MAX_MEMBER) {
		printf("%s\n", "Room member limitation reached! Can't join now!");
	}
	for (int i = 0; i < MAX_MEMBER; i++) {
		if (r->slave_socket[i] == 0)  {
			r->slave_socket[i] = fd;
			break;
		}
	}
	r->num_members += 1;
}

void remove_from_room(room* r, int fd) {
	for (int i = 0; i < MAX_MEMBER; i++) {
		if (r->slave_socket[i] == fd) { 
			r->slave_socket[i] = 0;
			r->num_members -= 1;
			break;
		}
	}
}
