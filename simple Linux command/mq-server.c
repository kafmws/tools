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
	int msg_type;
	char msg_text[MAX_TEXT];
};


int main() {
	struct msg_st senddata;
	int key, msgid;
	char buf[80];
	if((key=ftok(".",108)) == -1) {
		perror("ftok");
		exit(1);
	}
	if((msgid = msgget(key, IPC_CREAT|0666)) == -1) {
		perror("msgget");
		exit(1);
	}
	printf("%d\n", msgid);
	while(1) {
		printf("Enter the message to send:");
		fflush(stdin);
		scanf("%s", buf);
		senddata.msg_type=1;
		strcpy(senddata.msg_text, buf);

		if((msgsnd(msgid, (void *)&senddata, MAX_TEXT, 0))==-1) {
			perror("msgsnd");
			exit(1);
		}
		if(strcmp(buf, "bye")==0)	break;
	}
	return 0;
}
