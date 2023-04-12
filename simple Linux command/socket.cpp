#include<unistd.h>
#include<sys/socket.h> 

int main(){
	
	int listensocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	listen(listensocket, 10);
	return 0;
}
