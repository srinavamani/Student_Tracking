#include <stdio.h>

int main(int argc,char *argv[]){
//    FILE* file = popen("ntpdate", "r");
FILE* file = popen("aws s3 ls s3:// 2>&1", "r");
char buffer[1000];
//    fscanf(file, "%100s", buffer);
fgets(buffer, 1000, file);
pclose(file);
printf("buffer is :%s", buffer);
return 0;
}
