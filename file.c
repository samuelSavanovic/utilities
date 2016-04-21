#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

char* concat(int count, ...){
  va_list ap;
  int i;
  int len = 1;
  va_start(ap, count);
  for(i=0 ; i<count ; i++)
    len += strlen(va_arg(ap, char*));
  va_end(ap);
  char *merged = calloc(sizeof(char),len);
  int null_pos = 0;

  va_start(ap, count);
  for(i=0 ; i<count ; i++) {
    char *s = va_arg(ap, char*);
    strcpy(merged+null_pos, s);
    null_pos += strlen(s);
  }
  va_end(ap);
  return merged;
}


int main(int argc, char *argv[]) {
  struct stat sb;
  struct stat sb2;
  int c;
  char f_name[200], p_name[100], b_name[100];

  if (argc < 7) {
    printf("usage: file -p program name -f filename -b binary name\n");
    exit(1);
  }

  while((c = getopt(argc, argv, "f:p:b:")) != EOF) {
    switch(c) {
    case 'f':
      strcpy(f_name, optarg);
      break;
    case 'p':
      strcpy(p_name, optarg);
      break;
    case 'b':
      strcpy(b_name, optarg);
      break;
    case '?':
      exit(1);
      break;
    }
  }
  argv += optind;
  argc -= optind;

  if(access(f_name, F_OK) == -1) {
    perror(f_name);
    exit(1);
  }

  char * command = concat(7, p_name, " ", f_name," -o ", b_name," && ./", b_name);
  printf("Command: %s\n", command);

  while(1) {
    stat(f_name, &sb);
    sleep(1);
    stat(f_name, &sb2);
    if (sb.st_ctime != sb2.st_ctime){
      if (fork() == 0){
        if(!system(command)) {
          perror("exec");
          exit(1);
        }
        exit(0);
      }
      else
        wait(0);
    }
  }
  free(command);
  return 0;
}
