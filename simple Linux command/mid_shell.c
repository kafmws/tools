#include<pty.h>
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

#ifndef __USE_POSIX
#define __USE_POSIX
#endif
#include<limits.h>//for macro HOST_NAME_MAX

#ifdef _WIN64

#include<conio.h>
#define HOST_NAME_MAX 64

#else

#include<pwd.h>

#endif

#define check_base(exp, err, exit_status)\
	do{\
		if(exp){\
			perror(err);\
			exit(exit_status);\
		}\
	}while(0)

#define check(exp, err) check_base(exp,err,-1)

#define errat_base(exp, err, exit_status) check_base((exp),err,(exit_status))

#define errat(exp, err) check((exp),"error at "err"\n");

#define stderror(exp,fmt,...)\
	do{\
		if(exp){\
			fprintf(stderr,fmt" %s\n",##__VA_ARGS__,strerror(errno));\
		}\
	}while(0)

#ifndef _WIN64

#define is_shell_char(ch) ((ch)=='|'||(ch)=='>'||(ch)=='<')

//environment-copy
char 	cwd[];
char 	hostname[];
struct  passwd *pwd;

void tip() {
	stderror(getcwd(cwd,PATH_MAX) == NULL,"error at getcwd");
	if(strcmp(cwd,pwd->pw_dir) == 0) {
		cwd[0] = '~';
		cwd[1] = 0;
	}
	char * r_name = strrchr(cwd, '/');
	if(r_name && r_name[1] != 0) r_name++;
	else r_name = cwd;
	printf("myshell[%s@%s %s]$", pwd->pw_name, hostname, r_name);
}


#include <termio.h>

int getch() {
	struct termios tm, tm_old;
	int fd = 0, ch;
	if (tcgetattr(fd, &tm) < 0) {//保存现在的终端设置
		return -1;
	}
	tm_old = tm;
	cfmakeraw(&tm);//更改终端设置为原始模式，该模式下所有的输入数据以字节为单位被处理
	if (tcsetattr(fd, TCSANOW, &tm) < 0) {//应用更改之后的设置
		return -1;
	}
	ch = getchar();
	if (tcsetattr(fd, TCSANOW, &tm_old) < 0) {//还原设置
		return -1;
	}
	return ch;
}

#else

void tip() {
	printf("kafm>");
}

#endif


#define HISTORY_SIZE 256
#define CACHE_SIZE 20000
#define MAX_ARGS 256

//environment
char 	cwd[PATH_MAX] = {0};
char 	hostname[HOST_NAME_MAX] = {0};
char	history_filename[NAME_MAX+1] = {0};
FILE	*history_file = NULL;
struct  passwd *pwd = NULL;

//cmd history
int		c_p = 0;
char	cache[CACHE_SIZE + 1] = {0};
int		h_cnt = 0;
char	*history[HISTORY_SIZE] = {0};

//cmd
int  p = 0;
int	 arg_cnt = 0;
char *arg_list[MAX_ARGS]= {0};
int  cmds_cnt = 0;
int  exec_cnt = 0;
char *cmds_list[MAX_ARGS] = {0};
int  redirect_cnt = 0;
enum redirect_t {
    RDCT_IN, RDCT_OUT, RDCT_APPEND,
} redirect_type[MAX_ARGS];
char *redirect_list[MAX_ARGS] = {0};
int  pipes[2];
char buf[CACHE_SIZE];

//body
void set_shell_env();
void be_a_shell();
void split();
void parse();
void fork2exec();
int  lookup_shell_cmd();
void save_history();

//inner cmd
int   shell_cmd_cnt;
void (*shell_cmd[])();
char *shell_cmd_name[];
char *shell_cmd_helps[];

int main() {

	set_shell_env();
	be_a_shell();

	stderror(pipe(pipes),"error at pipe :");

	while(1) {

		tip();

		p = 0;
		int c, pre;
		int b = 0, hp = h_cnt;
		while(c = getch()) {
			if(p == CACHE_SIZE - 1) {
				printf("can not exceed %d characters, "
				       "the author is too lazy to use realloc.\n", CACHE_SIZE - 1);
			}
//			while(c!='a') {
//				printf("%d ",c);
//				c = getch();
//			}if(c == 'a')return 0;
			if(isprint(c)) {
				putchar(c);
				if(c != '\\') buf[p++] = c;
			} else if((c == 8 || c == 127) && p > b) { //backspace
				printf("\b \b");
				if(pre != '\\')--p;
			} else if(c == 13) {//'\n'
				if(pre == '\\' || pre == '|') {//just type a '\\' or '|'
					b = p;
					printf("\r\n> ");
				} else {
					printf("\r\n");
					break;
				}
			} else if(c == 27) { //escape
				getch();
				c = getch();
				if(c == 0x41) {//history up
					if(!hp) continue;

					if(hp == h_cnt) { //save the work
						while(h_cnt == HISTORY_SIZE || c_p + p + 2 > CACHE_SIZE )
							save_history(h_cnt>>1 + 1);
						memcpy(cache + c_p, buf + b, p - b);
						history[h_cnt] = cache + c_p;
						cache[c_p + p - b] = 0;
					}

					hp--;
					char *tbuf = buf;
					if(b == p) tbuf += p;
					else while(p>b && p--)printf("\b \b");
					strcpy(tbuf,history[hp]);
					p += printf("%s",tbuf);
//					if(b == p) {// just type a '\\'
//						strcpy(buf + p,history[hp]);
//						p += printf("%s",buf + p) - 1;
//					} else {
//						strcpy(buf,history[hp]);
//						while(p--)printf("\b");
//						p = printf("%s",buf) - 1;
//					}
				} else if(c == 0x42) {//history down
					if(hp>=h_cnt) continue;
					hp++;
					if(hp == h_cnt) {
						int t = strlen(history[hp - 1]);
						p -= t;
						while(t--)printf("\b \b");
						p += printf("%s",history[hp]);
						continue;
					}
					char *tbuf = buf;
					if(b == p) tbuf += p;
					else while(p>b && p--)printf("\b \b");
					strcpy(tbuf,history[hp]);
					p += printf("%s",tbuf);
				}
			} else if(c == 3) { //ctrl c | esc
				printf("\n");
				p = 0;
				break;
			}
			pre = c;
//			int ii = 0;
//			printf("\n");
//			while(ii<p) printf("%c",buf[ii++]);
//			printf("\np = %d\n",p);
		}
		buf[p] = 0;

		if(!p) continue;

		while(h_cnt == HISTORY_SIZE || c_p + p + 2 > CACHE_SIZE )
			save_history(h_cnt>>1 + 1);
		memcpy(cache + c_p,buf,p + 1);//include \0
		history[h_cnt++] = cache + c_p;
		c_p += p + 1;

		split();

		exec_cnt = 0;
		while(exec_cnt<cmds_cnt) {
			parse();

			/*print arg_list*/
//			printf("arg_cnt = %d\n",arg_cnt);
//			int i = 0;
//			for(i = 0; i<arg_cnt; i++) printf("(%s) ",arg_list[i]);

			if(arg_cnt == 0)
				continue;

			/*print redirect list*/
//#define RDCT(i) redirect_list[i<<1],redirect_list[(i<<1)+1]
//			printf("\nredirect:%d\n",redirect_cnt);
//			for(i = 0; i<redirect_cnt; i++) {
//				switch(redirect_type[i]) {
//					case RDCT_IN:
//						printf(" %s<%s ",RDCT(i));
//						break;
//					case RDCT_OUT:
//						printf(" %s>%s ",RDCT(i));
//						break;
//					case RDCT_APPEND:
//						printf(" %s>>%s ",RDCT(i));
//						break;
//				}
//			}
//			printf("\n");

			int call = lookup_shell_cmd(arg_list[0]);
			if(call!=-1) shell_cmd[call]();
			else fork2exec();
			exec_cnt++;
		}
	}
	return 0;
}

void split() {
	cmds_cnt = 0;
	cmds_list[cmds_cnt++] = buf;

	int cur = 0;
	while(cur<p) {
		if(buf[cur] == '|') {
			buf[cur] = 0;
			while(isspace(buf[++cur]));
			if(cur<p) cmds_list[cmds_cnt++] = buf + cur;
		}
		if(cmds_cnt == MAX_ARGS - 1) {
			printf("can not exceed %d args | pipes | redirect, "
			       "the author is too lazy to use realloc.\n", MAX_ARGS - 1);
			break;
		}
		cur++;
	}
	cmds_list[cmds_cnt] = buf + p;
	/*print the cmd split*/
//	printf("p = %d\n",p);
//	int i = 0;
//	for(i=0;i<cmds_cnt;i++) printf("(%s)",cmds_list[i]);
}

void parse() {

	arg_cnt = 0;
	redirect_cnt = 0;

	int  cur = 0;
	char *buf = cmds_list[exec_cnt];
	while(buf[cur]) {
		arg_list[arg_cnt++] = &buf[cur];//piece = &buf[cur]
		while(isgraph(buf[cur])) {
			if(buf[cur] == '>') {
				if(isspace(buf[cur-1])) arg_cnt--;
				redirect_type[redirect_cnt] = RDCT_OUT;
				buf[cur] = 0;
				int tcur = cur - 1;
				if(buf[++cur] == '>') {
					buf[++cur] = ' ';
					redirect_type[redirect_cnt] = RDCT_APPEND;
				}
				if(!isalpha(buf[tcur])) arg_cnt--;//not arg
				if(isdigit(buf[tcur])) { //parse the left redirect param
					while(tcur&&isdigit(buf[tcur])) tcur--;
					redirect_list[redirect_cnt << 1] = buf + tcur + 1;
				} else redirect_list[redirect_cnt << 1] = "1";
				while(isspace(buf[cur]))cur++;
				if(buf[cur] == 0) {
					arg_cnt = 0;//right redirect param not found
					fprintf(stderr,"redirect syntax error\n");
					break;
				}
				redirect_list[(redirect_cnt << 1) + 1] = buf + cur;
				while(isgraph(buf[cur]))cur++;
				redirect_cnt++;
				break;
			} else if(buf[cur] == '<') {
				if(isspace(buf[cur-1])) arg_cnt--;
				redirect_type[redirect_cnt] = RDCT_IN;
				break;
			}
			cur++;
		}
		if(buf[cur] == 0) break;
		buf[cur++] = 0;
		while(isspace(buf[cur]))cur++;
		if(arg_cnt == MAX_ARGS - 1 ||
		        redirect_cnt == (MAX_ARGS>>1) - 1) {
			printf("can not exceed %d args | pipes | redirect, "
			       "the author is too lazy to use realloc.\n", MAX_ARGS - 1);
			break;
		}
	}
	arg_list[arg_cnt] = NULL;
}

int lookup_shell_cmd(const char *cmd) {
	int i = 0;
	for(; i<shell_cmd_cnt; i++) {
		if(strcmp(cmd,shell_cmd_name[i])==0) {
			return i;
		}
	}
	return -1;
}

void fork2exec() {
	int p_return;
	int rdct_cnt = 0;
	//exec cmds_list
	pid_t pid = fork();
	errat(pid<0, "fork");
	if(pid == 0) {

		if(exec_cnt)
			stderror(dup2(pipes[0],0), "error at pipes:");
		if(exec_cnt != cmds_cnt - 1)
			stderror(dup2(pipes[1],1) != 1, "error at pipes");

		/*excute redirect*/
		while(rdct_cnt < redirect_cnt) {
			
			/*print the redirect*/
//				printf("re %d:\n",rdct_cnt);
//				switch(redirect_type[rdct_cnt]) {
//					case RDCT_IN:
//						printf(" %s<%s ",RDCT(rdct_cnt));
//						break;
//					case RDCT_OUT:
//						printf(" %s>%s ",RDCT(rdct_cnt));
//						break;
//					case RDCT_APPEND:
//						printf(" %s>>%s ",RDCT(rdct_cnt));
//						break;
//				}
//				printf("\n");

			int left = rdct_cnt<<1, right = left + 1;
			int oldfd = atoi(redirect_list[left]);
			stderror(close(oldfd),"error at close:");
			int newfd;
			if(redirect_list[right][0] == '&') {
				newfd = atoi(&redirect_list[right][1]);
			} else {
				if(redirect_type[rdct_cnt] == RDCT_IN) {
					newfd = open(redirect_list[right], O_RDONLY, 0644);
				} else if(redirect_type[rdct_cnt] == RDCT_OUT) {
					newfd = open(redirect_list[right], O_WRONLY|O_TRUNC|O_CREAT, 0644);
				} else if(redirect_type[rdct_cnt] == RDCT_APPEND) {
					newfd = open(redirect_list[right], O_APPEND|O_CREAT, 0644);
				}
			}
			stderror(newfd != oldfd, "error at redirect:");
			rdct_cnt++;
		}

		/*print actually cmd*/
		int tt = 0;
//		printf("arg_cnt = %d\n",arg_cnt);
//		printf("cmds:");
//		for(; tt < arg_cnt; tt++)
//			printf("(%s)",arg_list[tt]);
//		printf("\n");

		if(execvp(arg_list[0],arg_list) == -1) {
			exit(errno);
		}
	}
	else {
		wait(&p_return);
		if(p_return || errno) perror("");
	}

}

void set_shell_env() {
#ifndef _WIN64
	stderror(gethostname(hostname,HOST_NAME_MAX),"shell at gethostname :");
	pwd = getpwuid(getuid());
	stderror(getcwd(cwd,PATH_MAX) == NULL,"error at getcwd");
	if(strcmp(cwd,pwd->pw_dir) == 0) {
		cwd[0] = '~';
		cwd[1] = 0;
	}

	sprintf(history_filename,"%s/.kafm_shell_history",pwd->pw_dir);
	history_file = fopen(history_filename,"a");
	if(!history_file) {
		history_file = fopen(history_filename,"w");
		errat(history_file == NULL, "create");
	}
#endif

	//should identify the old record and newly
//	int i = 0;
//	FILE *fp = fopen(history_filename,"r");
//	for(;i<HISTORY_SIZE/2;i++){
//		if(~fscanf(fp,"%[^\n]",cache + c_p)){
//			history[h_cnt++] = cache + c_p;
//			c_p += strlen(cache + c_p) + 1;//add 1 for \0
//		}
//	}
//	fclose(fp);

	atexit(save_history);
}

void be_a_shell() {
	//kill(getppid(), SIGKILL);

//	pid_t pid = fork();
//	errat(pid<0, "fork");
//	if(pid > 0) exit(0);
//	pid_t sid = setsid();
//	errat_base(sid==-1,"setsid",errno);
//	printf("stdin tty: %s\n",ttyname(STDIN_FILENO));
//	int re = login_tty(STDIN_FILENO);
//	errat(re,"login_tty");
}

void save_history(int num) {
	if(num <= 0) num = h_cnt;
	if(num > h_cnt) num = h_cnt >> 1 + 1;//add 1 to avoid infinite loop
	int i = 0, len = 0;
	while(i<num) {
		//printf(" h: %s\n",history[i]);
		len += fprintf(history_file,"%s\n",history[i++]);
	}
	if(num == h_cnt) {
		h_cnt = 0;
		c_p = 0;
	} else {
		memcpy(cache, cache + len, c_p - len);
		c_p = c_p - len;
		h_cnt -= num;
	}
}

///////////////////////////////////////////////
static void shell_cmd_help() {
	if(arg_cnt == 1) {
helphelp:
		printf(
		    "kafm shell, version 0.1, just a toy.\n"
		    "These shell commands are defined internally.  Type `help' to see this list.\n"
		    "Type `help name' to find out more about the function `name'.\n\n"
		);
		printf(
		    "\thelp\t\tget help\t\t\t\texit\t\texit kafm shell\n"
		    "\thistory\t\tget command help\t\t\t\t\n"
		);
	} else if(arg_cnt == 2) {
		if(strcmp(arg_list[1],"help") == 0) goto helphelp;
		int call = lookup_shell_cmd(arg_list[1]);
		if(call == -1) {
			printf("no help topics match `%s'.  Try `help help' or `man -k %s' or `info %s'.",
			       arg_list[1], arg_list[1], arg_list[1], arg_list[1]);
		} else {
			printf("%s\n",shell_cmd_helps[call]);
		}
	}
}

static void shell_cmd_exit() {
	save_history(0);
	fclose(history_file);
	if(arg_cnt == 1) exit(0);
	else exit(atoi(arg_list[1]));
}


static void shell_cmd_history() {
//	fclose(history_file);
//	history_file = fopen(history_filename,"a");
	if(arg_cnt == 1) {
		FILE *fp = fopen(history_filename,"r");
		int i = 0;
		while(~fscanf(fp,"%[^\n]%*c",buf)) {
			printf("%5d  %s\n",++i,buf);
		}
		fclose(fp);
	}
}

static void shell_cmd_cd() {
	if(arg_cnt == 1) return;
	char *dir_name = arg_list[1];
	if(dir_name[0] == '~' && dir_name[1] == 0)
		dir_name = pwd->pw_dir;
	int re = chdir(dir_name);
	stderror(re,"cd: %s:",dir_name);
}

////////////////////////////////////////////////////
int shell_cmd_cnt = 4;

char *shell_cmd_name[] = {
	"help", "exit", "history", "cd"
};

char *shell_cmd_helps[] = {
	NULL, "usage:exit [status]\n exit kafm shell\n",
	"print the whole history stored the file .kafm_shell_history\n",
	"change current work path\n"
};

void (*shell_cmd[])() = {
	shell_cmd_help, shell_cmd_exit, shell_cmd_history,
	shell_cmd_cd
};