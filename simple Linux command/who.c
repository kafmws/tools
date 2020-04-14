#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<utmp.h>
#include<time.h>

#define exam(a,b)\
    do{\
        if((a) == (b)){\
            printf("文件打开失败\n");\
            if(errno == EACCES)printf("无权限\n");\
            printf("errno = %d\n",errno);\
            exit(1);\
        }\
    }while(0)

int main(int argc, char *argv[]){
    FILE *fp = fopen(UTMP_FILE, "r");
    exam(fp,NULL);
    struct utmp us;
    while(fread(&us,sizeof(struct utmp),1,fp)==1){
        if(us.ut_type != USER_PROCESS)continue;
        time_t ts = us.ut_time;
        char time[250] = { 0 };
        //strftime(time, 250, "%Y-%m-%d %H:%M", localtime(&ts));
        printf("%s\t %s\t      %s (%s)\n", us.ut_name, us.ut_line, time, us.ut_host);
    }
    fclose(fp);
    return 0;
}
