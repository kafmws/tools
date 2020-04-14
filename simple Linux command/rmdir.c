#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/types.h>

#define check(exp, err)\
	do{\
		if(exp){\
			perror(err);\
			exit(-1);\
		}\
	}while(0)

#define __S_IFMT 0xF000
#define __S_IFDIR 0x4000

void _remove(char *filename);

int main(int argc, char *argv[]) {
	
	check(	argc != 2, 
			"usage: rm filename\n"
			"like rm /dir/something\n"
		);
	
	char *filename = argv[1];
	check(access(filename, F_OK), "file not exists\n");
	
	DIR *dir = opendir(filename);
	check(!dir, "not a directory\n");
	
	int cnt = 0;
	struct dirent *file;
	while(file = readdir(dir)){
		if(strcmp(file->d_name,".")&&strcmp(file->d_name,"..")){
			cnt = 1;
			break;
		}
	}
	closedir(dir);
	
	if(cnt){
		char flag = 0;
		printf("directory is not empty, remove all files?(y/n):");
		flag = getchar();
		fflush(stdin);
		while(flag != 'y' && flag != 'n'){
			printf("y/n please:");
			flag = getchar();
			fflush(stdin);
		}
		if(flag == 'n') return 0;
	}
	_remove(filename);
	return 0;
}

void _remove(char *filename){
	DIR *dir = opendir(filename);
	check(!dir, "error at opendir\n");
	struct dirent *file;
	struct stat file_stat;
	while(file = readdir(dir)){
		if(!strcmp(file->d_name,".")||!strcmp(file->d_name,"..")) continue;
		char name[strlen(filename)+file->d_reclen+1];
		sprintf(name,"%s/%s",filename,file->d_name);
//		puts(name);
		check(stat(name,&file_stat),"error at stat\n");
		if((file_stat.st_mode & __S_IFMT)  == __S_IFDIR){
			_remove(name);
			rmdir(name);
		}else{
			check(unlink(name), "error at unlink\n");
		}
	}
	closedir(dir);
	rmdir(filename);
}
