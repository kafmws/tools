#include<time.h>
#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/types.h>

#ifndef _WIN64
#include<pwd.h>
#include<grp.h>
#endif

#define check(exp, err)\
	do{\
		if(exp){\
			perror(err);\
			exit(-1);\
		}\
	}while(0)

#ifdef _WIN64

#define S_IFMT   0170000    //文件类型的位mask
#define S_IFSOCK 0140000    //scoket    s
#define S_IFLNK 0120000     //符号连接  l
#define S_IFREG 0100000     //一般文件  -
#define S_IFBLK 0060000     //块设备    b
#define S_IFDIR 0040000     //目录      d
#define S_IFCHR 0020000     //字符设备  c
#define S_IFIFO 0010000     //管道文件  p

#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

#define chdir _chdir

//test
#define pw_name "kafm"
#define gr_name "kafm"
#define lstat stat

#endif

/*
*  dr-xr-xr-x. 5     root   root     4096 Nov  4 21:48 boot  inode
*  MODE        LINK  OWNER  GROUP  SIZE CHANGETIME   NAME    INODE
*/

#define LS_MODE 		FMTLEN[0]
#define LS_LINK 		FMTLEN[1]
#define LS_OWNER 		FMTLEN[2]
#define LS_GROUP 		FMTLEN[3]
#define LS_SIZE 		FMTLEN[4]
#define LS_CHANGETIME 	FMTLEN[5]
#define LS_NAME 		FMTLEN[6]
#define LS_INODE        FMTLEN[7]

#define TEM_SIZE 256
#define MODESTR_SIZE 12

static int FMTLEN[15] = {10,0,0,0,0,12};
static char *OPTION = NULL;

int filename_cmp(const void *a, const void *b) {
	return strcmp(*(char **)a,*(char **)b) > 0;
}

char *mode_to_str(char *str, mode_t mode) {
	int i;
	for(i = 0; i<MODESTR_SIZE-2; i++) str[i] = '-';

	//mutiple identify
//	switch(mode & S_IFMT){
//		case S_IFSOCK:str[0] = 's';break;
//		case S_IFLNK:str[0] = 'l';break;
//		case S_IFREG:str[0] = '-';break;
//		case S_IFBLK:str[0] = 'b';break;
//		case S_IFDIR:str[0] = 'd';break;
//		case S_IFCHR:str[0] = 'c';break;
//		case S_IFIFO:str[0] = 'p';break;
//		default: exit(-1);
//	}

//	printf("mode :0x%x,(mode & S_IFMT) = 0x%x\n",mode,(mode & S_IFMT));
	if((mode & S_IFMT) == S_IFSOCK) str[0] = 's';
	if((mode & S_IFMT) == S_IFREG) str[0] = '-';
	if((mode & S_IFMT) == S_IFLNK) str[0] = 'l';
	if((mode & S_IFMT) == S_IFBLK) str[0] = 'b';
	if((mode & S_IFMT) == S_IFDIR) str[0] = 'd';
	if((mode & S_IFMT) == S_IFCHR) str[0] = 'c';
	if((mode & S_IFMT) == S_IFIFO) str[0] = 'p';

	static unsigned	short permissions[10] = {
		0, S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP,
		S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH,
	};

	static char permission[11] = { ' ','r','w','x','r','w','x','r','w','x' };

	for(i = 1; i<10; i++) {
		if(mode & permissions[i]) str[i] = permission[i];
	}
	return str;
}

static void ls_info(char *filename) {

	struct stat fs;
	char modestr[MODESTR_SIZE] = {0};

	//use lstat
	check(lstat(filename,&fs), "error at lstat\n");
	if(OPTION['i']) {
		printf("%*d ",LS_INODE ,fs.st_ino);
	}
	if(OPTION['l']) {
#ifndef _WIN64
		struct passwd *pwd = getpwuid(fs.st_uid);
		check(pwd==NULL,"error at getpwuid\n");
		struct group *grp = getgrgid(fs.st_gid);
		check(grp==NULL,"error at getpwuid\n");
		char *pw_name = pwd->pw_name;
		char *gr_name = grp->gr_name;
#endif
		char time[25] = {0};
		time_t mtime = fs.st_mtime;
		struct tm *tm = localtime(&mtime);
		int len = strftime(time,25,"%b",tm);
		sprintf(time+len," %2d %02d:%02d",
		        tm->tm_mday,
		        tm->tm_hour,
		        tm->tm_min);
//		printf("%s %d %s %s %5d %s ",
//		       mode_to_str(modestr,fs.st_mode),
//		       fs.st_nlink,
//		       pw_name,
//		       gr_name,
//		       fs.st_size,
//		       time);
		printf("%-*s %*d %*s %*s %*d %*s ",
		       LS_MODE, mode_to_str(modestr,fs.st_mode),
		       LS_LINK, fs.st_nlink,
		       LS_OWNER, pw_name,
		       LS_GROUP, gr_name,
		       LS_SIZE, fs.st_size,
		       LS_CHANGETIME, time);
	}
	printf("%s ",filename);
	if(OPTION['l']&&modestr[0]!='l')printf("\n");
	if(modestr[0]=='l') { //linkfile
		char truename[PATH_MAX+1] = {0};
#ifndef _WIN64
		readlink(filename,truename,PATH_MAX);
#endif
		printf(" -> %s\n",truename);
	}
}

void scan_format_len(struct dirent *file) {
	unsigned t;
	struct stat fs;
	char tem[TEM_SIZE] = {0};
	if((t = strlen(file->d_name))>LS_NAME) LS_NAME = t;//file->d_reclen always zero
	if(OPTION['l'] || OPTION['i']) {
		check(lstat(file->d_name,&fs),"error at lstat");
	}
	if(OPTION['l']) {
		if((t = sprintf(tem,"%d",fs.st_nlink))>LS_LINK) LS_LINK = t;
#ifndef _WIN64
		struct passwd *pwd = getpwuid(fs.st_uid);
		check(pwd==NULL,"error at getpwuid\n");
		struct group *grp = getgrgid(fs.st_gid);
		check(grp==NULL,"error at getpwuid\n");
		if((t = strlen(pwd->pw_name))>LS_OWNER) LS_OWNER = t;
		if((t = strlen(grp->gr_name))>LS_GROUP) LS_GROUP = t;
#endif
		if((t = sprintf(tem,"%d",fs.st_size))>LS_SIZE) LS_SIZE = t;
	}
	if(OPTION['i']) {
		if((t = sprintf(tem,"%d",fs.st_ino))>LS_INODE) LS_INODE = t;
	}
}

static void ls(char *filename) {
	DIR *dir = opendir(filename);
	if(dir && !OPTION['d']) {

		chdir(filename);

		int file_cnt = 0;
		int tem_size = 50;
		char **d_name = malloc(sizeof(char *)*(tem_size));

		struct dirent *file;
		while(file = readdir(dir)) {
			if(file->d_name[0]!='.' || OPTION['a']) {
//				ls_info(file->d_name);
				scan_format_len(file);
				int len = strlen(file->d_name);
				d_name[file_cnt] = malloc(sizeof(char)*(len+1));
				memset(d_name[file_cnt],0,sizeof(char)*(len+1));
				memcpy(d_name[file_cnt],file->d_name,len);
				if(++file_cnt>=tem_size) {
					tem_size *= 1.5;
					d_name = realloc(d_name,sizeof(char *)*(tem_size));
				}
			}
		}

		int i;
		printf("total %d\n",file_cnt);

//		for(i = 0;i<file_cnt;i++) puts(d_name[i]);
//		puts("----------");
		qsort(d_name,file_cnt,sizeof(char *),filename_cmp);
//		for(i = 0;i<file_cnt;i++) puts(d_name[i]);
		for(i = 0; i<file_cnt; i++) {
			ls_info(d_name[i]);
		}
		closedir(dir);
	} else {
		ls_info(filename);
	}
}

int main(int argc, char *argv[]) {
#ifdef _WIN64
	if(argc <2) {
		argc = 3;
		char *argvs[] = {"","C:","-l"};
		argv = argvs;
	}
#endif

	static char help[] = 	"usage: ls [option] [filename]\n"
	                        "like ls -l -ia / \n\n"
	                        "-a   all file"
	                        "-h   print help\n"
	                        "-l   use a long listing format\n"
	                        "-i   print the index number of each file\n"
	                        "-d   list directories themselves\n";

	char **filename = malloc(sizeof(char *)*argc);
	memset(filename,0,sizeof(char *)*argc);
	char option['z' + 1] = {0};
	int i, cnt = 0;
	for(i = 1; i<argc; i++) {
		if(argv[i][0]=='-') {
			int j = 1;
			while(argv[i][j]) option[argv[i][j++]] = 1;
		} else filename[cnt++] = argv[i];
	}

	if(option['h']) {
		printf(help);
		return 0;
	}

	if(cnt == 0) filename[cnt++] = ".";

	qsort(filename,cnt,sizeof(char *),filename_cmp);

//	int line_cnt = 0, line;
//	if(option['i']) line = 5;
//	if(option['l']) line = 1;

	OPTION = option;
	for(i = 0; i<cnt; i++) {
		ls(filename[i]);
	}
	putchar('\n');
	free(filename);
//	for(i = 0;i<8;i++)printf("%d ",FMTLEN[i]);puts("");
	return 0;
}
