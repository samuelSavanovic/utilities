#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
struct string {
  char *ptr;
  size_t len;
};

void
init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t
writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

char*
concat(int count, ...){
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

void
download(CURL *curl, CURLcode *res, struct string *s,  char* category, char* resolution) {
  char *url = concat(4,"https://source.unsplash.com/category/",category, "/", resolution);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);
  *res = curl_easy_perform(curl);

}
size_t
write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void
downloadImg(CURL *curl, char* url, char* filename) {
    FILE *fp;
    fp = fopen(filename, "wb");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,fp);
    curl_easy_perform(curl);
    fsync(fp);
    fclose(fp);
}

int
main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(stderr, "usage: progName category resolution\nAvailable categories:\n\tbuildings food nature people technology objects\n\tResolution: WxH e.g 1600x900\n");
    exit(EXIT_FAILURE);
  }
  char buffer[30];
  struct timeval tv;
  time_t curtime;

  gettimeofday(&tv, NULL);
  curtime=tv.tv_sec;

  strftime(buffer,30,"%T.",localtime(&curtime));
  // printf("%s\n",buffer);
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  int ind1, ind2;
  char n[300];
  int j = 0;
  for (j; j < 300; j++)
    n[j] = -1;

  struct string s;

  if(curl) {
    init_string(&s);
    download(curl, &res, &s, argv[1], argv[2]);
    int i = 0;
    int fcount = 1;

    for (i; i < s.len; i++) {
      n[i] = s.ptr[i];
      if (s.ptr[i] == '\"') {
        if (fcount == 1) {
          ind1 = i;
          fcount ++;
        }
        else {
          ind2 = i;
        }
      }
    }
    curl_easy_cleanup(curl);
  }

  int i = ind1 +1;
  j = 0;
  char *new_url = (char*)malloc((ind2 - 2) * sizeof(char));
  for (j; j < ind2 - ind1 - 1; j++) {
    new_url[j] = n[i + j];
  }

  CURL* curl2 = curl_easy_init();
  downloadImg(curl2,  new_url, concat(3,"img", buffer, "jpg"));
  free(s.ptr);
  free(new_url);
  curl_easy_cleanup(curl2);
  return 0;
}
