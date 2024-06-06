#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define SHM_NAME "commands_shm"
#define SHM_SIZE 4096
#define MAX_COMMAND_LENGTH 256
#define OUTPUT_FILE "output.txt"

void writeOutput(char *command, char *output) {
    FILE *fp = fopen(OUTPUT_FILE, "a");
    if (fp == NULL) {
        perror("Error opening output file");
        exit(1);
    }
    fprintf(fp, "The output of: %s : is\n", command);
    fprintf(fp, ">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<\n", output);
    fclose(fp);
}

void read_commands(const char *filename, char *shared_memory) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening input file");
        exit(1);
    }
    
    char line[MAX_COMMAND_LENGTH];
    shared_memory[0] = '\0';
    while (fgets(line, sizeof(line), file)) {
        strcat(shared_memory, line);
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s sample_in_process.txt\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int shm_fd;
    char *shared_memory;
    pid_t pid;
    
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }
    
    shared_memory = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    pid = fork();
    if (pid == 0) {
        read_commands(filename, shared_memory);
        exit(0);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork");
        exit(1);
    }

    FILE *fp = fopen(OUTPUT_FILE, "w");
    if (fp == NULL) {
        perror("Error opening output file");
        exit(1);
    }
    fclose(fp);

    char *commands = strdup(shared_memory);
    char *command = strtok(commands, "\n");

    while (command != NULL) {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(1);
        }

        pid = fork();
        if (pid == 0) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[1]);

            char *args[MAX_COMMAND_LENGTH / 2 + 1];
            int i = 0;
            args[i] = strtok(command, " ");
            while (args[i] != NULL) {
                args[++i] = strtok(NULL, " ");
            }
            args[i] = NULL;

            printf("Executing command: ");
            for (int j = 0; j < i; j++) {
                printf("%s ", args[j]);
            }
            printf("\n");

            execvp(args[0], args);
            perror("execvp");
            exit(1);
        } else if (pid > 0) {
            close(pipefd[1]);

            char buffer[SHM_SIZE];
            ssize_t count = read(pipefd[0], buffer, sizeof(buffer) - 1);
            if (count == -1) {
                perror("read");
                exit(1);
            }
            buffer[count] = '\0';

            writeOutput(command, buffer);

            wait(NULL);
            command = strtok(NULL, "\n");
        } else {
            perror("fork");
            exit(1);
        }

        close(pipefd[0]);
    }

    if (munmap(shared_memory, SHM_SIZE) == -1) {
        perror("munmap");
        exit(1);
    }
    if (shm_unlink(SHM_NAME) == -1) {
        perror("shm_unlink");
        exit(1);
    }

    free(commands);
    return 0;
}
