#include <stdio.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <string.h> 
#include <stdlib.h>
 
#define BUFFER_SIZE 25000 
#define READ_END 0 
#define WRITE_END 1 

void word_count(int fd[2]){

}

int main(int argc, char **argv) 
{ 

if(argc !=2){
 printf("Please Provide the file name");
 return 0;
}
 pid_t pid; 

int fd[2];
char path[100];
int words=0; 
FILE * file;
char str[BUFFER_SIZE]; 

 /* create the pipe */ 
 if (pipe(fd) == -1) { 
  fprintf(stderr,"Pipe failed"); 
  return 1; 
 } 
 
 /* now fork a child process */ 
 pid = fork(); 
 
 if (pid < 0) { 
  fprintf(stderr, "Fork failed"); 
  return 1; 
 } 
 
 if (pid > 0) {  /* parent process */ 
 close(fd[READ_END]);
printf("\n Process 1 is reading file now..... \n");
file = fopen(path, "r");
   if (file == NULL){
      printf("\n Unable to open file. Please check the file name \n");
      exit(EXIT_FAILURE);
   }
printf("\n Process 1 starts sending data to Process 2 ... \n");
 while (fgets(str, BUFFER_SIZE, file) != NULL) {
	printf("\n %s \n",str)
        write(fd[WRITE_END], str, BUFFER_SIZE);
    }

 close(fd[WRITE_END]); 
 
 
 } 
 else { /* child process */ 
close(fd[WRITE_END]); 
int bytes;
 char inbuf[BUFFER_SIZE]; 
char ch;
int words=0;
while ((bytes = read(fd[0], inbuf, BUFFER_SIZE))>0){
      
      while(i<bytes){
	ch=inbuf[i];
   if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\0')
      words++;
	}
   }
printf("No of words is %d",words);
close(fd[READ_END]);
   
 } 
 
 return 0; 
} 
 