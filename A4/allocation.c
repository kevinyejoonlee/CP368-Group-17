#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 1048576 

typedef struct Hole {
    int start;
    int size;
    struct Hole *next;
} Hole;

typedef struct Process {
    int id;
    int start;
    int size;
    struct Process *next;
} Process;

Hole *holes = NULL;
Process *processes = NULL;

void initialize_memory(int size);
void allocate_memory(int process_id, int size, char algorithm);
void release_memory(int process_id);
void compact_memory();
void report_status();
int extract_process_id(char *str);
void execute_command(char *command);

int generate_unique_id(char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % 100000; 
}

void initialize_memory(int size) {
    holes = (Hole *)malloc(sizeof(Hole));
    holes->start = 0;
    holes->size = size;
    holes->next = NULL;
}

void allocate_memory(int process_id, int size, char algorithm) {
    Hole *best_hole = NULL, *best_prev = NULL;
    Hole *current = holes, *current_prev = NULL;

    while (current != NULL) {
        if (current->size >= size) {
            if (best_hole == NULL || (algorithm == 'B' && current->size < best_hole->size) ||
                (algorithm == 'W' && current->size > best_hole->size) ||
                (algorithm == 'F' && best_hole == NULL)) {
                best_hole = current;
                best_prev = current_prev;
            }
        }
        current_prev = current;
        current = current->next;
    }

    if (best_hole == NULL) {
        printf("No hole of sufficient size\n");
        return;
    }

    Process *new_process = (Process *)malloc(sizeof(Process));
    new_process->id = process_id;
    new_process->start = best_hole->start;
    new_process->size = size;
    new_process->next = processes;
    processes = new_process;

    best_hole->start += size;
    best_hole->size -= size;

    if (best_hole->size == 0) {
        if (best_prev == NULL) {
            holes = best_hole->next;
        } else {
            best_prev->next = best_hole->next;
        }
        free(best_hole);
    }

    printf("Successfully allocated %d to process P%d\n", size, process_id);
}

void release_memory(int process_id) {
    Process *current = processes, *prev = NULL;

    while (current != NULL && current->id != process_id) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Process not found\n");
        return;
    }

    Hole *new_hole = (Hole *)malloc(sizeof(Hole));
    new_hole->start = current->start;
    new_hole->size = current->size;
    new_hole->next = holes;
    holes = new_hole;

    if (prev == NULL) {
        processes = current->next;
    } else {
        prev->next = current->next;
    }
    free(current);

    printf("Successfully released memory for process P%d\n", process_id);
}

void compact_memory() {
    if (holes == NULL) {
        return;
    }

    int next_free_start = 0;
    Process *process = processes;
    Hole *new_hole = (Hole *)malloc(sizeof(Hole));
    new_hole->start = 0;
    new_hole->size = 0;
    new_hole->next = NULL;

    while (process != NULL) {
        if (process->start != next_free_start) {
            process->start = next_free_start;
        }
        next_free_start += process->size;
        process = process->next;
    }

    new_hole->start = next_free_start;
    new_hole->size = MAX - next_free_start;

    free(holes);
    holes = new_hole;

    printf("Compaction process is successful\n");
}

void report_status() {
    Hole *hole = holes;
    Process *process = processes;

    printf("Partitions [Allocated memory]:\n");
    while (process != NULL) {
        printf("Address [%d:%d] Process P%d\n", process->start, process->start + process->size - 1, process->id);
        process = process->next;
    }

    printf("Holes [Free memory]:\n");
    while (hole != NULL) {
        printf("Address [%d:%d] len = %d\n", hole->start, hole->start + hole->size - 1, hole->size);
        hole = hole->next;
    }
}

int extract_process_id(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }

    while (*str && !isdigit(*str)) {
        str++;
    }
    
    if (*str) {
        int id = atoi(str); // Convert string to integer
        return id;
    } else {
        return generate_unique_id(str); // Generate a unique ID
    }
}

void execute_command(char *command) {
    if (strncmp(command, "RQ", 2) == 0) {
        char process_str[10];
        int size;
        char algorithm;
        sscanf(command + 3, "%s %d %c", process_str, &size, &algorithm);
        int process_id = extract_process_id(process_str);
        if (process_id >= 0) {
            allocate_memory(process_id, size, algorithm);
        } else {
            printf("Invalid process ID\n");
        }
    } else if (strncmp(command, "RL", 2) == 0) {
        char process_str[10];
        sscanf(command + 3, "%s", process_str);
        int process_id = extract_process_id(process_str);
        if (process_id >= 0) {
            release_memory(process_id);
        } else {
            printf("Invalid process ID\n");
        }
    } else if (strcmp(command, "C") == 0) {
        compact_memory();
    } else if (strcmp(command, "Status") == 0) {
        report_status();
    } else {
        printf("Invalid command\n");
    }
}

int main(int argc, char *argv[]) {
    char command[50];

    if (argc != 2) {
        printf("Usage: %s <memory size>\n", argv[0]);
        return 1;
    }

    int memory_size = atoi(argv[1]);
    printf("Initializing memory with size: %d bytes\n", memory_size);

    initialize_memory(memory_size);

    while (1) {
        printf("allocator> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0'; 

        if (strcmp(command, "Exit") == 0) {
            break;
        }

        execute_command(command);
    }

    return 0;
}
