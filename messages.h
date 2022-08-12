#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 256

int findSize(char dirname[], char file_name[]);
char **createMessage1(char dirname[], int* num_files, int* maxFileSize);
char **createMessage2(char dirname[], int num_files, int maxFileSize);
void createFiles(char dirname[], char** message1, int num_files);
void populateFiles(char dirname[], char** message1, char** message2, int num_files);

#endif