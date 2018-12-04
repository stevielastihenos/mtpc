#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include "time_functions"
#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

#define MAXSIZE 10
#define linesize 1024

FILE * fpointer1;
FILE * fpointer2;

void* threadOne(void* in_file);
void* threadTwo(void* out_file);
void path();

char * bufferArray[MAXSIZE];

sem_t mutex1;
sem_t mutex2;
sem_t mutex3;

int i = 0;
int k = 0;

int main(void) {

  path();

  sem_init(&mutex1, 0 , 0);           // semaphores initialized to later be used inside
  sem_init(&mutex2, 0 , MAXSIZE);     // thread functions to ensure the bufferarray is being
  sem_init(&mutex3, 0 , 1);           // read to and written from properly

  pthread_t thread1;
  pthread_t thread2;
  pthread_attr_t attr1;
  pthread_attr_t attr2;
  pthread_attr_init(&attr1);
  pthread_attr_init(&attr2);

  start_nanotime();

  pthread_create(&thread1, &attr1, threadOne, (void*)fpointer1);
  pthread_create(&thread2, &attr2, threadTwo, (void*)fpointer2);

  stop_timing();

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  sem_destroy(&mutex1);
  sem_destroy(&mutex2);
  sem_destroy(&mutex3);

  printf("time is %d", get_nanodiff());

  return 0;
}

void* threadOne(void* in_file) {

  FILE* fpointer1;
  fpointer1 = (FILE*)in_file;

  while(!(feof(fpointer1))) { // while file is not empty
    char singleLine[linesize];
    fgets(singleLine, linesize, fpointer1); // get line from fpointer1 and place into singleLine

      sem_wait(&mutex2);
      sem_wait(&mutex3);
    bufferArray[i] = singleLine; // put line in current buffer index
	  i = (i+1) % MAXSIZE; // mod needed for circular array
      sem_post(&mutex3);
      sem_post(&mutex1);
  }

    sem_wait(&mutex2);
    sem_wait(&mutex3);
  bufferArray[i] = "^^\n"; // put special code into buffer when file is empty
	i = (i+1) % MAXSIZE;
    sem_post(&mutex3);
    sem_post(&mutex1);

  fclose(fpointer1);
  pthread_exit(NULL);
}

void* threadTwo(void* out_file)  {

  FILE* fpointer2;
  fpointer2 = (FILE*)out_file;
  char* lineOut;

  while(1) {        // ensures writing to output file until the special code is detected

      sem_wait(&mutex1);
      sem_wait(&mutex3);
    lineOut = bufferArray[k]; // write line from the buffer to the output file
    k = (k+1) % MAXSIZE;

    if(strcmp(lineOut, "^^\n") != 0) {      //used to break once the special code is read in the buffer
      fprintf(fpointer2, "%s", lineOut);
      fflush(fpointer2);
    }
    else {
      break;
    }
      sem_post(&mutex3);
      sem_post(&mutex2);
  }
  fclose(fpointer2);
  pthread_exit(NULL);
}

void path() {         // setpath and definitions .h files were not working, therefore created this path func inorder to set directories accordingly

#ifdef _WIN32
  char in_path[]="C:\\temp\\coursein\\p2-in.txt";
  char out_path[]="C:\\temp\\courseout\\p2-out.txt";

#else
  char in_path[200], out_path[200];
  const char *homedir;
  homedir = getenv("HOME");
  if (homedir!= NULL) {
    homedir = getpwuid(getuid())->pw_dir;
  }
  printf("my home directory is: %s\n", homedir);
  strcpy(in_path,homedir);
  strcpy(out_path,homedir);
  strcat(in_path,"/tmp/coursein/p2-in.txt");
  strcat(out_path,"/tmp/courseout/p2-out.txt");

#endif
  fpointer1 = fopen(in_path, "r");
  fpointer2 = fopen(out_path, "w");

  if ((fpointer1 == NULL) || (fpointer2 == NULL)) {
    printf("Error...r/ The files were unable to be opened\n");
    exit(1);
  }

}
