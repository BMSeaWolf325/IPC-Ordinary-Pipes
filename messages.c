#include "messages.h"

void populateFiles(char dirname[], char** message1, char** message2, int num_files)
{
    for (int i = 0; i < num_files; i++)
    {
        int fd;
        int file_path_len = 2 + strlen(dirname) + 1 + strlen(message1[i]) + 1;
        char file_path[file_path_len];
        snprintf(file_path, file_path_len, "./%s/%s", dirname, message1[i]); // creates a file path to file i in dirname
        fd = open(file_path, O_WRONLY); // opens file i
        if (fd == -1)
        {
            fprintf(stderr, "Opening File Failed.\n");
            exit(1);
        }
        // write the contents of message2[i] to file i
        if (write(fd, message2[i], strlen(message2[i])) == -1)
        {
            fprintf(stderr, "Writing to File Failed.\n");
            exit(1);
        }
        if(close(fd) == -1)
        {
            fprintf(stderr, "Closing File Failed.\n");
            exit(1);
        }        
    }
}

void createFiles(char dirname[], char** message1, int num_files)
{
    for (int i = 0; i < num_files; i++)
    {
        int fd;
        int file_path_len = 2 + strlen(dirname) + 1 + strlen(message1[i]) + 1;
        char file_path[file_path_len];
        snprintf(file_path, file_path_len, "./%s/%s", dirname, message1[i]); // creates a file path to file i in dirname
        fd = open(file_path, O_CREAT, S_IRWXU); // creates file i with read, write, execute perms for the owner of the file
        if (fd == -1)
        {
            fprintf(stderr, "Opening File Failed.\n");
            exit(1);
        }
        if(close(fd) == -1)
        {
            fprintf(stderr, "Closing File Failed.\n");
            exit(1);
        }
    }
}

char **createMessage2(char dirname[], int num_files, int maxFileSize)
{
    DIR *directory;
    struct dirent *entry;
    char direct_path[3 + strlen(dirname)];
    snprintf(direct_path, 3 + strlen(dirname), "./%s", dirname); // creates a path to dirname
    directory = opendir(direct_path); // opens the directory

    if (directory == NULL)
    {
        fprintf(stderr, "Opening Directory Failed.\n");
        exit(1);
    }

    int currentFile = 0;
    char **message2;
    // allocate memory for message 2
    message2 = (char**)malloc(sizeof(char*) * num_files); // creates array of char* of length num_files
    for (int i = 0; i < num_files; i++)
    {
        message2[i] = (char*)malloc(sizeof(char) * maxFileSize); // only allocates size of each char* to the maximum file size in order to conserve memory
    }

    while ((entry = readdir(directory)) != NULL) // while the end of the directory is not reached
    {
        if (entry->d_type == DT_REG) // if current entry is of type file
        {
            int fd;
            int file_path_len = 2 + strlen(dirname) + 1 + strlen(entry->d_name) + 1;
            char file_path[file_path_len];
            snprintf(file_path, file_path_len, "./%s/%s", dirname, entry->d_name); // create file path to current file
            fd = open(file_path, O_RDONLY); // open the file in read only mode
            if (fd == -1)
            {
                fprintf(stderr, "Opening File Failed.\n");
                exit(1);
            }
            // read the contents of the file and store them in message2[currentFile]
            if (read(fd, message2[currentFile], findSize(dirname,entry->d_name)) == -1)
            {
                fprintf(stderr, "Reading from File Failed.\n");
                exit(1);
            }
            if(close(fd) == -1)
            {
                fprintf(stderr, "Closing File Failed.\n");
                exit(1);
            }
            currentFile += 1;
        }
    }

  
    if (closedir(directory) == -1) // close the directory
    {
        fprintf(stderr, "Closing Directory Failed.\n");
        exit(1);
    }

    return message2; // return the double pointer as message2
}

char **createMessage1(char dirname[], int* num_files, int* maxFileSize)
{
    DIR *directory;
    struct dirent *entry;
    char direct_path[3 + strlen(dirname)];
    snprintf(direct_path, 3 + strlen(dirname), "./%s", dirname); // creates a path to dirname
    directory = opendir(direct_path); // opens the directory

    if (directory == NULL)
    {
        fprintf(stderr, "Opening Directory Failed.\n");
        exit(1);
    }
    while ((entry = readdir(directory)) != NULL) // while the end of the directory is not reached
    {
        if (entry->d_type == DT_REG) // if current entry is of type file
        {
            *num_files += 1; // increment the number of files in this directory
            int res = findSize(dirname,entry->d_name); // find the size of the file in bytes
            if (res > *maxFileSize) // update the directory's maxFileSize if the current file size is bigger
            {
                *maxFileSize = res;
            }
        }
    }

    rewinddir(directory); // rewind entry to point at the beginning

    int currentFile = 0;
    char **message1;
    message1 = (char**)malloc(sizeof(char*) * *num_files); // creates array of char* of length num_files

    while ((entry = readdir(directory)) != NULL) // while the end of the directory is not reached
    {
        if (entry->d_type == DT_REG) // if current entry is of type file
        {
            message1[currentFile] = strdup(entry->d_name); // sets message1[currentFile] to a pointer of a new string which is a duplicate of entry->d_name
            currentFile += 1;
        }
    }

    if (closedir(directory) == -1) // close the directory
    {
        fprintf(stderr, "Closing Directory Failed.\n");
        exit(1);
    }

    return message1; // return the double pointer as message2
}

int findSize(char dirname[], char file_name[])
{
    char direct_path[BUFFER_SIZE];
    snprintf(direct_path, BUFFER_SIZE, "./%s/%s", dirname, file_name); // creates a path to the file_name

    FILE* fp = fopen(direct_path, "r");
  
    if (fp == NULL)
    {
        fprintf(stderr, "Opening File Failed.\n");
        exit(1);
    }
  
    fseek(fp, 0L, SEEK_END); // sets fp to the end of the file

    int fileSize = ftell(fp); // current value of fp, which is the size of the file since fp is pointing to the end of the file
  
    if (fclose(fp) != 0) // close the file
    {
        fprintf(stderr, "Closing File Failed.\n");
        exit(1);
    }
  
    return fileSize; // return the size of the file
}