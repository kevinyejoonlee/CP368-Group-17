#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define SHM_NAME "collatz_shm"
#define MAX_SEQUENCE_LENGTH 1000

// odd *3+1
// even /2
void generate_collatz_sequence(int n, int *sequence, int *length) {
    
    *length = 0;
    while (n != 1) {
        sequence[(*length)++] = n;
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            n = 3 * n + 1;
        }
    }
    sequence[(*length)++] = n;
}

void child_process() {
    int shm_fd;
    int *shared_memory;

    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    shared_memory = mmap(0, MAX_SEQUENCE_LENGTH * sizeof(int), PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    printf("Child Process: The generated collatz sequence is ");
    for (int i = 0; shared_memory[i] != 1; i++) {
        printf("%d ", shared_memory[i]);
    }
    printf("1\n");

    if (munmap(shared_memory, MAX_SEQUENCE_LENGTH * sizeof(int)) == -1) {
        perror("munmap");
        exit(1);
    }
    close(shm_fd);
    exit(0); 
}

void parent_process(int start_number) {
    int shm_fd;
    int *shared_memory;
    int sequence[MAX_SEQUENCE_LENGTH];
    int length;

    generate_collatz_sequence(start_number, sequence, &length);

    // shm_open(), ftruncate(), and mmap()
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, MAX_SEQUENCE_LENGTH * sizeof(int));
    shared_memory = mmap(0, MAX_SEQUENCE_LENGTH * sizeof(int), PROT_WRITE, MAP_SHARED, shm_fd, 0);

    for (int i = 0; i < length; i++) {
        shared_memory[i] = sequence[i];
    }

    pid_t pid = fork();
    if (pid == 0) {
        child_process();
    } else if (pid > 0) {
        wait(NULL); 
        if (munmap(shared_memory, MAX_SEQUENCE_LENGTH * sizeof(int)) == -1) {
            perror("munmap");
            exit(1);
        }
        if (shm_unlink(SHM_NAME) == -1) {
            perror("shm_unlink");
            exit(1);
        }
    } else {
        perror("fork");
        exit(1);
    }
}

// TXT MUST HAVE BLANK LINE AT THE END FOR THIS TO WORK OR IT WILL INF LOOP!!!
int main(int argc, char *argv[]) {
    FILE *file;
    int start_number;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s start_numbers.txt\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "r");

    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }


    while (fscanf(file, "%d", &start_number) != EOF ) {

        if (fscanf(file, "%d", &start_number) == 0){
            break;
        }
        printf("Parent Process: The positive integer read from file is: %d\n", start_number);
        parent_process(start_number);

     
    }
    fclose(file);
    return 0;
}
