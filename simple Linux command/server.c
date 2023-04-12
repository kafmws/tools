#include<ctype.h>
#include<errno.h>
#include<stdio.h>
#include<fcntl.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>

#define stderror(exp,fmt,...)\
	do{\
		if(exp){\
			fprintf(stderr,fmt" %s\n",##__VA_ARGS__,strerror(errno));\
		}\
	}while(0)

init_sem(int semid,int semnum,int val) {
	union semun {
		int val;
		struct semid_ds *buf;
		unsigned short *array;
	} initval;
	initval.val = val;
	if(semctl(semid,semnum,SETVAL,initval)==-1)    {
		perror("semctl");
		exit(1);
	}
}

lockforwrite(int semid){
    struct sembuf action[2];
    action[0].sem_num=0;//read 
    action[0].sem_flg=SEM_UNDO;
    action[0].sem_op=-1;
    action[1].sem_num=1;//write
    action[1].sem_flg=SEM_UNDO;
    action[1].sem_op=0;
    if(semop(semid,action,2)==-1)    {
		perror("semop");
		exit(1);
    }
}

unlockafterwrite(int semid){
    struct sembuf action[1];
    action[0].sem_num=0;
    action[0].sem_flg=SEM_UNDO;
    action[0].sem_op=+1;
    if(semop(semid,action,1)==-1)    {
		perror("semop");
		exit(1);
    }
}

int main() {
	char buf[50];
	FILE *file = fopen("..datefile","r");
	while(1) {
		scanf("%s",buf);
		puts(buf);
		sleep(1000);
	}
	return 0;
}
