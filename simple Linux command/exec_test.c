#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>

int main(int argc, char *argv[]){
	
	if(argc!=2){
		printf("usage: surf destination_website\n"
			   "like surf www.bing.com\n");
		exit(EXIT_FAILURE);
	}
	
	int status,info;
	char *description = "";
	pid_t pid = fork();
	if(pid<0){
		perror("error at fork\n");
		exit(EXIT_FAILURE);
	}else if(pid == 0){
		status = execvp("links",(char* []){"links",argv[1],NULL});
//		status = execvp("links",argv);
		if(status == -1) exit(EXIT_FAILURE);
	}else {
		wait(&status);
		printf("child_pid = %u\n",pid);
		if(WIFEXITED(status)){
			printf("normal exit");
			info = WEXITSTATUS(status);
		}else if(WIFSIGNALED(status)){
			printf("abnormal exit");
			info = WTERMSIG(status);
		}else if(WIFSTOPPED(status)){
			printf("process stop");
			info = WSTOPSIG(status);
		}else {
			printf("unknown status\n");
			exit(0);
		} 
		printf(", more info:%d\n",info);
	}
	return 0;
}
