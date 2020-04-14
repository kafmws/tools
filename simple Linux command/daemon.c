#include<time.h>
#include<utmp.h>
#include<stdio.h>
#include<fcntl.h>
#include<errno.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<ulimit.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/resource.h>

void user_snapshot() {
	FILE *fp = fopen(UTMP_FILE, "r");
	FILE *file = fopen("/run/log/daemon_log","wa");
	if(fp == NULL || file == NULL) {
		perror("error at fopen\n");
		exit(EXIT_FAILURE);
	}
	struct utmp us;
	char time[250] = { 0 };
	while(fread(&us,sizeof(struct utmp),1,fp)==1) {
		if(us.ut_type != USER_PROCESS)continue;
		time_t ts = us.ut_time;
		strftime(time, 250, "%Y-%m-%d %H:%M", localtime(&ts));
		fprintf(file, "%s\t %s\t      %s (%s)\n", us.ut_name, us.ut_line, time, us.ut_host);
	}
	fclose(fp);
	fclose(file);
}

int main(int argc, char *argv[]) {
	
	if(argc!=2){
		perror("usage: daemon hh:mm\n");
		exit(EXIT_SUCCESS);
	}
	
	signal(SIGHUP,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);

	pid_t pid = fork();
	if(pid < 0) {
		perror("error at fork\n");
		exit(EXIT_FAILURE);
	}
	if(pid > 0) exit(EXIT_SUCCESS);
	setsid();

	pid = fork();
	if(pid < 0) {
		perror("error at fork\n");
		exit(EXIT_FAILURE);
	}
	if(pid > 0) exit(EXIT_SUCCESS);

	struct rlimit rl;
	int i, file_limit = getrlimit(RLIMIT_NOFILE, &rl);
	for(i = 0; i<file_limit; i++) close(i);

	chdir("/");

	umask(0);

	signal(SIGCHLD,SIG_IGN);

	open("/dev/null",O_RDONLY);
	open("/dev/null",O_RDWR);
	open("/dev/null",O_RDWR);

	int hour,min,all;
	sscanf(argv[1],"%d:%d",&hour,&min);
	all = (hour * 60 + min);
	//printf("hour = %d  min = %d\n",hour,min);

	time_t t;
	struct tm *s_time;
	while(1) {
		time(&t);
		s_time = localtime(&t);
		//printf("chour = %d  cmin = %d\n",s_time->tm_hour,s_time->tm_min);
		if(s_time->tm_hour == hour
		    && s_time->tm_min == min) {
			user_snapshot();
			//printf("86430\n");
			sleep(86340);
		} else {
			int gap = (all - (s_time->tm_hour*60 + s_time->tm_min) + 24*60) % (24*60);
			//printf("sleep %d\n",gap*60);
			sleep(gap * 60);
		}
	}
	return 0;
}
