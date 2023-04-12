#include<ctype.h>
#include<errno.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<unistd.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/stat.h>
#include<sys/types.h>

#define MAX_TEXT 512


struct msg_st {
	long msg_type;
	char msg_text[MAX_TEXT];
};


int main() {
	struct msg_st data;
	int key, msgid;
	if((key=ftok(".",108)) == -1) {
		perror("ftok");
		exit(1);
	}
	if((msgid = msgget(key, IPC_CREAT|0666)) == -1) {
		perror("msgget");
		exit(1);
	}
//	printf("msgid:%d\n", msgid);
	do {
		if((msgrcv(msgid, (void *)&data, MAX_TEXT, 0, 0))==-1) {
			perror("msgsnd");
			exit(1);
		}
		if(data.msg_type == 1) {
			printf("receive: %s\n", data.msg_text);
		}
	} while(strcmp(data.msg_text, "bye"));
	msgctl(msgid,IPC_RMID,NULL);
	return 0;
}
