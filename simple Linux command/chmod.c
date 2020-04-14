#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>

#define check(exp, err)\
	do{\
		if(exp){\
			perror(err);\
			exit(-1);\
		}\
	}while(0)

#define S_IRGRP    (S_IRUSR>>3)
#define S_IWGRP   (S_IWUSR>>3)
#define S_IXGRP    (S_IXUSR>>3)

#define S_IROTH     (S_IRGRP>>3)
#define S_IWOTH    (S_IWGRP>>3)
#define S_IXOTH     (S_IXGRP>>3)

#define USR_KIND 3
#define PMS_KIND 3

int main(int argc, char *argv[]) {

	static char 	usr[] = {'u', 'g', 'o', 0};
	static char 	rwx[] = {'r', 'w', 'x', 0};
	static unsigned short masks[][USR_KIND] = {
		{ S_IREAD, S_IWRITE, S_IEXEC },
		{ S_IRGRP, S_IWGRP, S_IXGRP },
		{ S_IROTH, S_IWOTH, S_IXOTH },
	};
	

//argc = 3;
//char *args[] = {"C:\\Users\\kafm\\Desktop\\chmod.exe", "o-rw" ,"test.c"};
//argv = args;
//printf("%d\n",argc);
//for(int i = 0;i<argc;i++)puts(argv[i]);

	check(	argc != 3
	        || argv[1][0] != 'u'
	        && argv[1][0] != 'g'
	        && argv[1][0] != 'o',
			"usage: chomd [ugo][+-=][rwx]\n"
			"like chomd u=rwx\n"
		);

	char *cl = argv[1];
	char *filename = argv[2];

	check(access(filename, F_OK), "file not exists.\n");

	struct stat file_stat;
	check(stat(filename, &file_stat), "error at stat.\n");

	//parse cl
	size_t uindex = 0;
	while(isalpha(cl[uindex]))uindex++;
	
	size_t op = uindex;
	size_t pms = op + 1;
	
	int usrs[USR_KIND] = { 0 };
	size_t index = 0;
	for(;index<uindex;index++){
		size_t i = 0;
		for(;usr[i];i++){
			if(usr[i]==cl[index]){
				usrs[i] = 1;
				break;
			}
		}
		if(!usr[i]) goto chmod_error;
	}
	
	unsigned short m = 0;
	while(cl[pms]){
		int j = 0;
		while(j<PMS_KIND&&rwx[j]!=cl[pms])j++;
		if(j>=PMS_KIND) goto chmod_error;
		int i = 0;
		for(;i<USR_KIND;i++){
			if(usrs[i]) m = m | masks[i][j];
		}
		pms++;
	}
	
	unsigned short mode = file_stat.st_mode;
	if(cl[op] == '+'){
		mode |= m;
	}else if(cl[op] == '-'){
		mode &= ((~m)|0xFE00);
	}else if(cl[op] == '='){
		int i = 0;
		for(;i<USR_KIND;i++){
			if(usrs[i]){
				mode = mode & (0xFFFFE3F>>(i*3)) | (m & (0x0001C0>>(i*3)));
			}
		}
	}else {
chmod_error:
		perror("usage: chomd [ugo][+-=][rwx]\n"
			   "like chomd u=rwx\n");
		exit(-1);
	}
	
	check(chmod(filename, mode), "chmod failed\n");
	return 0;
}
