#include <sys/wait.h>
#include "messages.h"

#define READ_END 0
#define WRITE_END 1

int main(void)
{
    int pipe1[2]; // file descriptor for pipe 1 (sends msg from child 1 to child 2)
    int pipe2[2]; // file descriptor for pipe 2 (sends msg from child 2 to child 1)
    pid_t child1; // process id for child 1
    pid_t child2; // process id for child 2

    // create pipe 1
    if (pipe(pipe1) == -1)
    {
        fprintf(stderr,"Pipe 1 Creation Failed.\n");
        exit(1);
    }

    // create pipe 2
    if (pipe(pipe2) == -1)
    {
        fprintf(stderr,"Pipe 2 Creation Failed.\n");
        exit(1);
    }

    // fork child 1
    child1 = fork();
    if (child1 == -1)
    {
        fprintf(stderr, "Child 1 Fork Failed.\n");
        exit(1);
    }
    else if (child1 == 0) // inside child 1
    {
        // close the unused pipe ends
        if (close(pipe1[READ_END]) == -1)
        {
            fprintf(stderr,"Closing Read End of Pipe 1 Failed.\n");
            exit(1);
        }
        if (close(pipe2[WRITE_END]) == -1)
        {
            fprintf(stderr,"Closing Write End of Pipe 2 Failed.\n");
            exit(1);
        }

        char dirname[BUFFER_SIZE] = "d1"; // name of the directory child 1 is responsible for
        int num_filesd1 = 0, maxFileSized1 = 0; // number of files and max file size of d1
        int num_filesd2 = 0, maxFileSized2 = 0; // number of files and max file size of d2
        char **message1d1, **message1d2; // message with the names of the files of d1 and d2
        char **message2d1, **message2d2; // message with the contents of the files of d1 and d2

        // initialize message1d1, message2d1,  num_filesd1, and maxFileSized1
        message1d1 = createMessage1(dirname, &num_filesd1, &maxFileSized1);
        message2d1 = createMessage2(dirname, num_filesd1, maxFileSized1);

        // write number of files in d1 to child 2
        if (write(pipe1[WRITE_END], &num_filesd1, sizeof(int)) == -1)
        {
            fprintf(stderr,"Error Writing to Pipe 1.\n");
            exit(1);
        }
        // read number of files in d2 from child 2
        if (read(pipe2[READ_END], &num_filesd2, sizeof(int)) == -1)
        {
            fprintf(stderr,"Error Reading from Pipe 2.\n");
            exit(1);            
        }

        // write max file size in d1 to child 2
        if (write(pipe1[WRITE_END], &maxFileSized1, sizeof(int)) == -1)
        {
            fprintf(stderr,"Error Writing to Pipe 1.\n");
            exit(1);
        }
        // read max file size in d2 from child 2
        if (read(pipe2[READ_END], &maxFileSized2, sizeof(int)) == -1)
        {
            fprintf(stderr,"Error Reading from Pipe 2.\n");
            exit(1);            
        }

        // allocate memory for message1d2 and message2d2
        message1d2 = (char**)malloc(sizeof(char*) * num_filesd2);
        for (int i = 0; i < num_filesd2; i++)
        {
            message1d2[i] = (char*)malloc(sizeof(char) * BUFFER_SIZE);
        }
        message2d2 = (char**)malloc(sizeof(char*) * num_filesd2);
        for (int i = 0; i < num_filesd2; i++)
        {
            message2d2[i] = (char*)malloc(sizeof(char) * maxFileSized2);
        }

        // write message1d1 to child 2
        for (int i = 0; i < num_filesd1; i++)
        {
            if (write(pipe1[WRITE_END], message1d1[i], sizeof(char) * BUFFER_SIZE) == -1)
            {
                fprintf(stderr,"Error Writing to Pipe 1.\n");
                exit(1);                
            }
        }
        // read message1d2 from child 2
        for (int i = 0; i < num_filesd2; i++)
        {
            if (read(pipe2[READ_END], message1d2[i], sizeof(char) * BUFFER_SIZE) == -1)
            {
                fprintf(stderr,"Error Reading from Pipe 2.\n");
                exit(1);                
            }
        }
        
        // create the files listed by child 2 in d1
        createFiles(dirname, message1d2, num_filesd2);

        // write message2d1 to child 2 
        for (int i = 0; i < num_filesd1; i++)
        {
            if (write(pipe1[WRITE_END], message2d1[i], sizeof(char) * maxFileSized1) == -1)
            {
                fprintf(stderr,"Error Writing to Pipe 1.\n");
                exit(1);                
            }
        }
        // read message2d2 from child 2
        for (int i = 0; i < num_filesd2; i++)
        {
            if (read(pipe2[READ_END], message2d2[i], sizeof(char) * maxFileSized2) == -1)
            {
                fprintf(stderr,"Error Reading from Pipe 2.\n");
                exit(1);                
            }
        }

        // populate the files listed by child 2 in d1
        populateFiles(dirname, message1d2, message2d2, num_filesd2);

        // free the allocated memory for message1d1, message2d1, message1d2, message2d2
        for (int i = 0; i < num_filesd1; i++)
        {
            free(message1d1[i]);
            free(message2d1[i]);
            message1d1[i] = NULL;
            message2d1[i] = NULL;
        }
        free(message1d1);
        free(message2d1);
        message1d1 = NULL;
        message2d1 = NULL;
        for (int i = 0; i < num_filesd2; i++)
        {
            free(message1d2[i]);
            free(message2d2[i]);
            message1d2[i] = NULL;
            message2d2[i] = NULL;
        }
        free(message1d2);
        free(message2d2);
        message1d2 = NULL;
        message2d2 = NULL;

        // close the opened ends of the pipes
        if (close(pipe1[WRITE_END]) == -1)
        {
            fprintf(stderr,"Closing Write End of Pipe 1 Failed.\n");
            exit(1);
        }
        if (close(pipe2[READ_END]) == -1)
        {
            fprintf(stderr,"Closing Read End of Pipe 2 Failed.\n");
            exit(1);
        }
        return 0; // terminate child 1
    }

    // fork child 2
    child2 = fork();
    if (child2 == -1)
    {
        fprintf(stderr, "Child 2 Fork Failed.\n");
        exit(1);
    }
    else if (child2 == 0) // inside child 2
    {
        // close the unused pipe ends
        if (close(pipe1[WRITE_END]) == -1)
        {
            fprintf(stderr,"Closing Write End of Pipe 1 Failed.\n");
            exit(1);
        }
        if (close(pipe2[READ_END]) == -1)
        {
            fprintf(stderr,"Closing Read End of Pipe 2 Failed.\n");
            exit(1);
        }

        char dirname[BUFFER_SIZE] = "d2"; // name of the directory child 2 is responsible for
        int num_filesd1 = 0, maxFileSized1 = 0; // number of files and max file size of d1
        int num_filesd2 = 0, maxFileSized2 = 0; // number of files and max file size of d2
        char **message1d1, **message1d2; // message with the names of the files of d1 and d2
        char **message2d1, **message2d2; // message with the contents of the files of d1 and d2
        
        // initialize message1d1, message2d1,  num_filesd1, and maxFileSized1
        message1d2 = createMessage1(dirname, &num_filesd2, &maxFileSized2);
        message2d2 = createMessage2(dirname, num_filesd2, maxFileSized2);

        // write number of files in d2 to child 1
        if (write(pipe2[WRITE_END], &num_filesd2, sizeof(int)) == -1)
        {
            fprintf(stderr,"Error Writing to Pipe 2.\n");
            exit(1);
        }
        // read number of files in d1 from child 1
        if (read(pipe1[READ_END], &num_filesd1, sizeof(int)) == -1)
        {
            fprintf(stderr,"Error Reading from Pipe 1.\n");
            exit(1);            
        }
        
        // write max file size in d2 to child 1
        if (write(pipe2[WRITE_END], &maxFileSized2, sizeof(int)) == -1)
        {
            fprintf(stderr,"Error Writing to Pipe 2.\n");
            exit(1);
        }
        // read max file size in d1 from child 1
        if (read(pipe1[READ_END], &maxFileSized1, sizeof(int)) == -1)
        {
            fprintf(stderr,"Error Reading from Pipe 1.\n");
            exit(1);            
        }

        // allocate memory for message1d1 and message2d1
        message1d1 = (char**)malloc(sizeof(char*) * num_filesd1);
        for (int i = 0; i < num_filesd2; i++)
        {
            message1d1[i] = (char*)malloc(sizeof(char) * BUFFER_SIZE);
        }
        message2d1 = (char**)malloc(sizeof(char*) * num_filesd1);
        for (int i = 0; i < num_filesd1; i++)
        {
            message2d1[i] = (char*)malloc(sizeof(char) * maxFileSized1);
        }

        // write message1d2 to child 1
        for (int i = 0; i < num_filesd2; i++)
        {
            if (write(pipe2[WRITE_END], message1d2[i], sizeof(char) * BUFFER_SIZE) == -1)
            {
                fprintf(stderr,"Error Writing to Pipe 2.\n");
                exit(1);                
            }
        }
        // read message1d1 from child 1
        for (int i = 0; i < num_filesd1; i++)
        {
            if (read(pipe1[READ_END], message1d1[i], sizeof(char) * BUFFER_SIZE) == -1)
            {
                fprintf(stderr,"Error Reading from Pipe 1.\n");
                exit(1);                
            }
        }
        
        // create the files listed by child 1 in d2
        createFiles(dirname, message1d1, num_filesd1);

        // write message2d2 to child 1
        for (int i = 0; i < num_filesd1; i++)
        {
            if (write(pipe2[WRITE_END], message2d2[i], sizeof(char) * maxFileSized2) == -1)
            {
                fprintf(stderr,"Error Writing to Pipe 2.\n");
                exit(1);                
            }
        }
        // read message2d1 from child 1
        for (int i = 0; i < num_filesd2; i++)
        {
            if (read(pipe1[READ_END], message2d1[i], sizeof(char) * maxFileSized1) == -1)
            {
                fprintf(stderr,"Error Reading from Pipe 1.\n");
                exit(1);                
            }
        }

        // populate the files listed by child 1 in d2
        populateFiles(dirname, message1d1, message2d1, num_filesd1);

        // free the allocated memory for message1d1, message2d1, message1d2, message2d2
        for (int i = 0; i < num_filesd1; i++)
        {
            free(message1d1[i]);
            free(message2d1[i]);
            message1d1[i] = NULL;
            message2d1[i] = NULL;
        }
        free(message1d1);
        free(message2d1);
        message1d1 = NULL;
        message2d1 = NULL;
        for (int i = 0; i < num_filesd2; i++)
        {
            free(message1d2[i]);
            free(message2d2[i]);
            message1d2[i] = NULL;
            message2d2[i] = NULL;
        }
        free(message1d2);
        free(message2d2);
        message1d2 = NULL;
        message2d2 = NULL;
        
        // close the opened ends of the pipes
        if (close(pipe2[WRITE_END]) == -1)
        {
            fprintf(stderr,"Closing Write End of Pipe 2 Failed.\n");
            exit(1);
        }
        if (close(pipe1[READ_END]) == -1)
        {
            fprintf(stderr,"Closing Read End of Pipe 1 Failed.\n");
            exit(1);
        }
        return 0; // terminate child 2
    }

    // close the unused pipes in the parent
    if (close(pipe1[READ_END]) == -1)
    {
        fprintf(stderr,"Closing Read End of Pipe 1 Failed");
        exit(1);
    }
    if (close(pipe1[WRITE_END]) == -1)
    {
        fprintf(stderr,"Closing Write End of Pipe 1 Failed");
        exit(1);
    }
    if (close(pipe2[READ_END]) == -1)
    {
        fprintf(stderr,"Closing Read End of Pipe 2 Failed");
        exit(1);
    }
    if (close(pipe2[WRITE_END]) == -1)
    {
        fprintf(stderr,"Closing Write End of Pipe 2 Failed");
        exit(1);
    }

    // wait for both child processes to terminate
    for (int i = 0; i < 2; i++) {
        wait(NULL);
    }
    return 0; // terminate parent
}