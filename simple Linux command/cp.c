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

#define MASKS 0777
#define BUF_SIZE 40960

void copy_file(char *src, char *dst);

void file_to_dir(const char *, char *);

void dir_to_dir(char *src, char *dst);

void copy(char *src, char *dst);

int main(int argc, char *argv[]) {

#ifdef _WIN64
	argc = 3;
	char *args[] = {"cp.exe",
	                "dir",
	                "newDir"
	               };
	argv = args;
#define mkdir(a,b) mkdir(a)
#endif

	check(	argc != 3,
	        "usage: cp src dst\n"
	        "like cp /file_src /new_dir/file_dst\n"
	     );

	//note: check wheather src contains dst or not

	char *src_filename = argv[1];
	char *dst_filename = argv[2];

	check(access(src_filename, F_OK), "source file not exists.\n");
	copy_file(src_filename, dst_filename);
	return 0;
}

const char *relative_path(const char *path) {
	char *p = strrchr(path, '/');
	if(!p) p = strrchr(path, '\\');
	if(p) return p + 1;
	return path;
}

void file_to_dir(const char *src_filename, char *dst_filename) {
	char *pre = dst_filename;
	char *name = strrchr(src_filename,'/');
	if(!name)name = strrchr(src_filename,'\\');
	name = name ? name + 1 : src_filename;
	int filename_len = strlen(name);
	int dir_len = strlen(dst_filename);
	int len = dir_len + filename_len + 2;
	check((dst_filename = malloc(len)) == NULL, "error at malloc");
	memcpy(dst_filename,pre,dir_len);
	dst_filename[dir_len] = '/';
	memcpy(dst_filename + dir_len + 1, name, filename_len);
	dst_filename[len - 1] = 0;
	copy(src_filename, dst_filename);
	free(dst_filename);
}

void dir_to_dir(char *src, char *dst) {
	DIR *dir = NULL;
	if(dir = opendir(src)) {
		struct dirent *file = NULL;
		while(file = readdir(dir)) {
			if(strcmp(file->d_name,".")&&strcmp(file->d_name,"..")) {
				char filename[strlen(src)+file->d_reclen+2];
				sprintf(filename,"%s/",src);
				memcpy(filename + strlen(src) + 1,file->d_name,file->d_reclen);
				filename[strlen(src)+file->d_reclen+1] = 0;
//				printf("filename %s\n",filename);
				if(opendir(filename)) { //dir
					char new_dst[strlen(dst)+file->d_reclen+2];
					sprintf(new_dst,"%s/",dst,file->d_name);
					memcpy(new_dst + strlen(dst) + 1,file->d_name,file->d_reclen);
					new_dst[strlen(dst)+file->d_reclen+1] = 0;
//					printf("mkdir %s\n",new_dst);
					check(mkdir(new_dst,MASKS),"error at mkdir");
//					printf("dir to dir: %s, %s\n",file->d_name, new_dst);
					dir_to_dir(filename,new_dst);
					continue;
				}
				char name[strlen(src)+file->d_reclen+2];
				sprintf(name,"%s/%s",src,file->d_name);
//				printf("copy_file %s, %s\n",name, dst);
				copy_file(name,dst);
			}
		}
	}
}

void copy_file(char *src_filename, char *dst_filename) {
	struct stat src_stat;
	check(stat(src_filename,&src_stat), "error at stat.\n");
	if((src_stat.st_mode & __S_IFMT)  == __S_IFDIR) {
		if(access(dst_filename, F_OK))
			check(mkdir(dst_filename,MASKS),"error at mkdir");
		dir_to_dir(src_filename, dst_filename);
		return;
	}

	struct stat dst_stat;
	if(access(dst_filename, F_OK) == 0) {
		check(stat(dst_filename,&dst_stat), "error at stat.\n");
		if((dst_stat.st_mode & __S_IFMT) == __S_IFDIR) {
			file_to_dir(src_filename, dst_filename);
			return;
		} else {//
			printf("destination file is already exists, override?(y/n):");
			check(getchar()!='y',"");
		}
	}
	copy(src_filename,dst_filename);
}

void copy(char *src, char *dst) {
	struct stat src_stat;
	check(stat(src,&src_stat),"error at stat");
	int fd_src = open(src,O_RDONLY);
	int fd_dst = open(dst,O_WRONLY | O_CREAT,MASKS, S_IRWXU);
	check(fd_src < 0 || fd_dst < 0, "error at open\n");
	int cnt = 0;
	char buf[BUF_SIZE];
	while(cnt=read(fd_src,buf,BUF_SIZE)) {
		write(fd_dst,buf,cnt);
	}
	check(chmod(dst,src_stat.st_mode),"error at chmod");
}

