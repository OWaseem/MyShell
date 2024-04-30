#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAX_ARGS 50

typedef struct
{
    char *args[MAX_ARGS];
    char *args1[MAX_ARGS];
    char *args2[MAX_ARGS];
    int redirection_input1;
    int redirection_output1;
    int redirection_input2;
    int redirection_output2;
    int redirection_pipe;
    char *inputfile;
    char *outputfile;
}Full_Args;

void execute_with_pipe_pipe(char *args1[], char *args2[], int input1, int output1, int input2, int output2, int redir_pipe, 
                       char *inputfile, char *outputfile) 
{

    int fd[2];

    if(pipe(fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t child = fork();

    if(child == -1)
    {
        perror("child");
        exit(EXIT_FAILURE);
    }

    if(child == 0)
    {   
        char filepathlocal[512];
        char filepathusr[512];
        char filepathbin[512];
        snprintf(filepathlocal, sizeof(filepathlocal), "/usr/local/bin/%s", args1[0]);
        snprintf(filepathusr, sizeof(filepathusr), "/usr/bin/%s", args1[0]);
        snprintf(filepathbin, sizeof(filepathbin), "/bin/%s", args1[0]);

        close(fd[0]);

        if(input1 == 1)
        {
            int inputfd1 = open(inputfile, O_RDONLY, 0640);
            if(inputfd1 == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(inputfd1, STDIN_FILENO);
            close(inputfd1);
        }

        if(output1 == 1)
        {
            int output_fd1 = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if(output_fd1 == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(output_fd1, STDOUT_FILENO);
            close(output_fd1);
        }
        else
        {
            dup2(fd[1], STDOUT_FILENO);
        }

        close(fd[1]);

        if(access(filepathlocal, F_OK) == 0)
        {
            execv(filepathlocal, args1);
            perror("execv");
            exit(EXIT_FAILURE);
        }
        else if(access(filepathusr, F_OK) == 0)
        {
            execv(filepathusr, args1);
            perror("execv");
            exit(EXIT_FAILURE);
        }

        else if(access(filepathbin, F_OK) == 0)
        {
            execv(filepathbin, args1);
            perror("execv");
            exit(EXIT_FAILURE);
        }
        
    }

    
    
    // next process
    pid_t child2 = fork();
    if(child2 == -1)
    {
        perror("child");
        exit(EXIT_FAILURE);
    }

    if(child2 == 0)
    {
        char filepathlocal[512];
        char filepathusr[512];
        char filepathbin[512];
        snprintf(filepathlocal, sizeof(filepathlocal), "/usr/local/bin/%s", args2[0]);
        snprintf(filepathusr, sizeof(filepathusr), "/usr/bin/%s", args2[0]);
        snprintf(filepathbin, sizeof(filepathbin), "/bin/%s", args2[0]);

        close(fd[1]);

        if(input2 == 1)
        {
            int inputfd2 = open(inputfile, O_RDONLY, 0640);
            if(inputfd2 == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(inputfd2, STDIN_FILENO);
            close(inputfd2);
        }
        else
        {
            dup2(fd[0], STDIN_FILENO);
        }

        if(output2 == 1)
        {
            int output_fd2 = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if(output_fd2 == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(output_fd2, STDOUT_FILENO);
            close(output_fd2); 
        }
        
        close(fd[0]);

        if(access(filepathlocal, F_OK) == 0)
        {
            execv(filepathlocal, args2);
            perror("execv");
            exit(EXIT_FAILURE);
        }
        else if(access(filepathusr, F_OK) == 0)
        {
            execv(filepathusr, args2);
            perror("execv");
            exit(EXIT_FAILURE);
        }

        else if(access(filepathbin, F_OK) == 0)
        {
            execv(filepathbin, args2);
            perror("execv");
            exit(EXIT_FAILURE);
        }
            
    } 

    close(fd[1]);
    waitpid(child2, NULL, 0);
    waitpid(child, NULL, 0);
    // Read from the pipe
    char output[5000];
    ssize_t nbytes = read(fd[0], output, sizeof(output));
    if (nbytes > 0) {
        printf("%.*s", (int)nbytes, output);
    }
    close(fd[0]); // Close read end of the pipe
}

void execute_with_pipe_input_output(char *args[], char *inputfile, char* outputfile) {

    char filepathlocal[512];
    char filepathusr[512];
    char filepathbin[512];
    snprintf(filepathlocal, sizeof(filepathlocal), "/usr/local/bin/%s", args[0]);
    snprintf(filepathusr, sizeof(filepathusr), "/usr/bin/%s", args[0]);
    snprintf(filepathbin, sizeof(filepathbin), "/bin/%s", args[0]);

    int fd[2];
    pipe(fd);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        close(fd[0]); // Close read end of the pipe

        // Redirect stdin to read from the input file
        int input_fd = open(inputfile, O_RDONLY, 0640);
        if (input_fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(input_fd);

        // Redirect stdout to the write end of the pipe
        int output_fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
        if(output_fd == -1)
        {
            perror("open");
            exit(EXIT_FAILURE);
        }
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(output_fd);
        close(fd[1]); // Close write end of the pipe

        // Execute the command
        if(access(filepathlocal, F_OK) == 0)
        {
            execv(filepathlocal, args);
            perror("execv");
            exit(EXIT_FAILURE);
        }
        else if(access(filepathusr, F_OK) == 0)
        {
            execv(filepathusr, args);
            perror("execv");
            exit(EXIT_FAILURE);
        }

        else if(access(filepathbin, F_OK) == 0)
        {
            execv(filepathbin, args);
            perror("execv");
            exit(EXIT_FAILURE);
        }

    } else {
        // Parent process
        close(fd[1]); // Close write end of the pipe

        // Wait for the child process to complete
        waitpid(pid, NULL, 0);

        close(fd[0]); // Close read end of the pipe
    }
}


void execute_with_pipe_output(char *args[], char *outputfile) {
    char filepathlocal[512];
    char filepathusr[512];
    char filepathbin[512];
    snprintf(filepathlocal, sizeof(filepathlocal), "/usr/local/bin/%s", args[0]);
    snprintf(filepathusr, sizeof(filepathusr), "/usr/bin/%s", args[0]);
    snprintf(filepathbin, sizeof(filepathbin), "/bin/%s", args[0]);

    int fd[2];
    pipe(fd);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        close(fd[0]); // Close read end of the pipe
        int output_fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
        if(output_fd == -1)
        {
            perror("open");
            exit(EXIT_FAILURE);
        }
        // Redirect stdout to write to the output file
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
        close(fd[1]);

        // Redirect stdin to read from the read end of the pipe

        // Execute the command
        if(access(filepathlocal, F_OK) == 0)
        {
            execv(filepathlocal, args);
            perror("execv");
            exit(EXIT_FAILURE);
        }
        else if(access(filepathusr, F_OK) == 0)
        {
            execv(filepathusr, args);
            perror("execv");
            exit(EXIT_FAILURE);
        }

        else if(access(filepathbin, F_OK) == 0)
        {
            execv(filepathbin, args);
            perror("execv");
            exit(EXIT_FAILURE);
        }
    } 
    else {
        // Parent process
        close(fd[1]); // Close read end of the pipe

        
        // Wait for the child process to complete
        close(fd[0]); // Close read end of the pipe
        waitpid(pid, NULL, 0);
        
    }
}

void execute_with_pipe(char *args[]) {

    char filepathlocal[512];
    char filepathusr[512];
    char filepathbin[512];
    snprintf(filepathlocal, sizeof(filepathlocal), "/usr/local/bin/%s", args[0]);
    snprintf(filepathusr, sizeof(filepathusr), "/usr/bin/%s", args[0]);
    snprintf(filepathbin, sizeof(filepathbin), "/bin/%s", args[0]);
    int fd[2];
    pipe(fd);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        close(fd[0]); // Close read end of the pipe

        // Redirect stdout to the write end of the pipe
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]); // Close write end of the pipe

        if(access(filepathlocal, F_OK) == 0)
        {
            execv(filepathlocal, args);
            perror("execv");
            exit(EXIT_FAILURE);
        }
        else if(access(filepathusr, F_OK) == 0)
        {
            execv(filepathusr, args);
            perror("execv");
            exit(EXIT_FAILURE);
        }

        else if(access(filepathbin, F_OK) == 0)
        {
            execv(filepathbin, args);
            perror("execv");
            exit(EXIT_FAILURE);
        }
        
    } else {
        // Parent process
        close(fd[1]); // Close write end of the pipe


        // Wait for the child process to complete
        waitpid(pid, NULL, 0);

        // Read from the pipe
        char output[5000];
        ssize_t nbytes = read(fd[0], output, sizeof(output));
        if (nbytes > 0) {
            printf("%.*s", (int)nbytes, output);
        }
        close(fd[0]); // Close read end of the pipe
    }
}

int redirection(char *args[], int arg) {
    
    Full_Args data = {
    .redirection_input1 = 0,
    .redirection_output1 = 0,
    .redirection_input2 = 0,
    .redirection_output2 = 0,
    .redirection_pipe = 0,
    .inputfile = NULL,
    .outputfile = NULL,
    .args[MAX_ARGS-1] = NULL,
    .args1[MAX_ARGS-1] = NULL,
    .args2[MAX_ARGS-1] = NULL
};

    int data_arg_index = 0; // Index for data.args
    int pipe_index = 0; // Index for where | starts

    for(int i = 0; i < arg; i++)
    {
        if(strcmp(args[i], "<") == 0) // input redirection
        {
            data.inputfile = args[i+1];
            if(data.redirection_pipe == 1)
            {
                if(i < pipe_index)
                {
                    data.redirection_input1 = 1;
                }
                else
                {
                    data.redirection_input2 = 1;
                }
            }
            else
            {
                data.redirection_input1 = 1;
            }
            i++; //skip inputfile argument
        }
        else if(strcmp(args[i],">") == 0) // output redirection
        {
            data.outputfile = args[i+1];

            if(data.redirection_pipe == 1)
            {
                if(i < pipe_index)
                {
                    data.redirection_output1 = 1;
                }
                else
                {
                    data.redirection_output2 = 1;
                }
            }
            else
            {
                data.redirection_output1 = 1;
            }
            i++; //skip outputfile argument
        }
        else if(strcmp(args[i],"|") == 0)
        {
            data.redirection_pipe = 1;
            data.args[data_arg_index++] = args[i];
            pipe_index = i;
        }
        else
        {
            data.args[data_arg_index++] = args[i];
        }
    }

    // Terminate data.args with NULL
    data.args[data_arg_index] = NULL;

    if(data.redirection_pipe == 1)
    {
        int data_arg1_index = 0; // Index for data.args1
        int data_arg2_index = 0; // Index for data.args2
        int pipe_index2 = 0; // Index for where | starts

        for(int i = 0; i < data_arg_index; i++)
        {
            if(strcmp(data.args[i], "|") == 0)
            {
                pipe_index2 = i;
            }
        }

        for(int i = 0; i < pipe_index2; i++)
        {
            data.args1[i] = data.args[i];
            data_arg1_index++;
        }

        // Terminate data.args1 with NULL
        data.args1[data_arg1_index] = NULL;

        for(int i = pipe_index2+1; i < data_arg_index; i++)
        {
            data.args2[data_arg2_index] = data.args[i];
            data_arg2_index++;
        }

        // Terminate data.args1 with NULL
        data.args2[data_arg2_index] = NULL;
    }

    if(data.redirection_input1 == 1 && data.redirection_output1 == 1)
    {
        execute_with_pipe_input_output(data.args, data.inputfile, data.outputfile);
    }
    else if(data.redirection_input1 == 1)
    {
        execute_with_pipe(data.args);
    }
    else if(data.redirection_output1 == 1)
    {
        execute_with_pipe_output(data.args, data.outputfile);
    }
    else if(data.redirection_pipe == 1)
    {
        execute_with_pipe_pipe(data.args1, data.args2, data.redirection_input1, data.redirection_output1, data.redirection_input2, 
                      data.redirection_output2, data.redirection_pipe, data.inputfile, data.outputfile);
    }
    else
    {
        execute_with_pipe(data.args);
    }


    return EXIT_SUCCESS;
}

int match(const char *pattern, const char *text) {
    // Wildcard matching algorithm
    while (*pattern && *text) {
        if (*pattern == '*') {
            while (*pattern == '*')
                pattern++;
            if (*pattern == '\0')
                return 1;  // Pattern ends with '*', so it matches
            while (*text) {
                if (match(pattern, text))
                    return 1;  // Pattern after '*' matches
                text++;
            }
            return 0;  // No match found after '*'
        } else if (*pattern == '?' || *pattern == *text) {
            pattern++;
            text++;
        } else {
            return 0;  // Characters do not match
        }
    }
    return (*pattern == *text);
}

void expand_wildcards(char *input) {
    char expanded[MAX_ARGS * 256]; // Maximum size after expansion
    char *output = expanded;
    char *token = strtok(input, " ");

    while (token != NULL) {
        if (strchr(token, '*')) {
            // Wildcard found, expand it
            char *pattern = token;
            char *directory_path = ".";
            DIR *dir = opendir(directory_path);
            if (dir != NULL) {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    if (match(pattern, entry->d_name)) {
                        output += sprintf(output, "%s ", entry->d_name);
                    }
                }
                closedir(dir);
            }
        } else {
            // No wildcard, copy the token as is
            output += sprintf(output, "%s ", token);
        }
        token = strtok(NULL, " ");
    }

    // Copy the expanded string back to the input buffer
    strcpy(input, expanded);
}

int main(int argc, char *argv[]) {
    int redir_pipe = 0;
    char *args[MAX_ARGS];
    int arg = 0;
    char buffer[1000];
    char s[256];
    char filepathlocal[512];
    char filepathusr[512];
    char filepathbin[512];
    while(1) {
        const char* text = "mysh> ";
        write(STDOUT_FILENO, text, strlen(text));
        ssize_t input = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (input == -1) {
            perror("read");
            return EXIT_FAILURE;
        }
        
        buffer[strcspn(buffer, "\n")] = '\0';
        expand_wildcards(buffer);

        char *token = strtok(buffer, "\" ");

        while (token != NULL) {

        // Handle quoted arguments
        if (token[0] == '"') {
            // If the token starts with a quote, keep concatenating tokens until the closing quote is found
            args[arg++] = token + 1; // Skip the opening quote
            token = strtok(NULL, "\" ");
        } else {
            
            args[arg++] = token;
            token = strtok(NULL, "\" ");
        }
    }

    if (buffer[input - 1] == '\n') {
            buffer[input - 1] = '\0';
        }
        
        
        snprintf(filepathlocal, sizeof(filepathlocal), "/usr/local/bin/%s", args[0]);
        snprintf(filepathusr, sizeof(filepathusr), "/usr/bin/%s", args[0]);
        snprintf(filepathbin, sizeof(filepathbin), "/bin/%s", args[0]);

    if(strcmp(args[0], "cd") == 0) 
    {
        if(arg == 1) {
            if(chdir("..") != 0)
            {
                perror("cd");
            }
        }
        else
        {
            if (chdir(args[1]) == -1){
            perror("cd");
        }

        }
        
    } 
    else if (strcmp(args[0] , "exit") == 0) 
    {
        exit(EXIT_SUCCESS);
    } 
    else if (strcmp(args[0], "pwd") == 0) 
    {
        if(getcwd(s, sizeof(s)) == NULL) {
            perror("getcwd");
            return EXIT_FAILURE;
        }
        printf("%s\n", s);
    }

    else if(strcmp(args[0] , "which") == 0)
    {
        snprintf(filepathlocal, sizeof(filepathlocal), "/usr/local/bin/%s", args[1]);
        snprintf(filepathusr, sizeof(filepathusr), "/usr/bin/%s", args[1]);
        snprintf(filepathbin, sizeof(filepathbin), "/bin/%s", args[1]);

        if(access(filepathlocal, F_OK) == 0)
        {
            printf("%s\n", filepathlocal);
        }
        else if(access(filepathusr, F_OK) == 0)
        {
            printf("%s\n", filepathusr);
        }
        else if(access(filepathbin, F_OK) == 0)
        {
            printf("%s\n", filepathbin);
        }
        else
        {
            printf("Invalid command or command not given");
        }
    }



    else if(access(filepathlocal, F_OK) == 0 || access(filepathusr, F_OK) == 0 || access(filepathbin, F_OK) == 0) 
    {
    if (arg < 1) {
        printf("No command specified.\n");
    } 
    else {
        // Construct argument array for execution
        char *command_args[MAX_ARGS];
        for (int i = 0; i < arg; i++) {
            command_args[i] = args[i];
        }
        command_args[arg] = NULL; // Terminate argument list

        if(arg < 2)
        {
            execute_with_pipe(command_args);
        }
        else
        {
            redirection(command_args, arg);
        }
    }

    }

    else
    {
        printf("command not found");
    }

    arg = 0;
    printf("\n");
    }

    // for(int i = 0; i < arg; i++)
    // {
    //     free(args[i]);
    // }

    return EXIT_SUCCESS;
}