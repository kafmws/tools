#include<stdio.h>

int main(){
	char str[100];
	scanf("%[^\n]%*c", str);
	printf("%s",str);
	perror("test error");
	return 0;
}
