#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

void copy_file(const char *input_file, const char *output_file) {
    int input_fd, output_fd;
    ssize_t read_bytes, write_bytes;
    char buffer[4096];

    // Open input file for reading
    input_fd = open(input_file, O_RDONLY);
    if (input_fd == -1) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    // Create and open output file for writing (truncate if exists)
    output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        perror("Error opening/creating output file");
        close(input_fd);
        exit(EXIT_FAILURE);
    }

    // Copy the contents of the input file to the output file
    while ((read_bytes = read(input_fd, buffer, sizeof(buffer))) > 0) {
        write_bytes = write(output_fd, buffer, read_bytes);
        if (write_bytes != read_bytes) {
            perror("Error writing to output file");
            close(input_fd);
            close(output_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (read_bytes == -1) {
        perror("Error reading from input file");
    }

    // Close files
    close(input_fd);
    close(output_fd);

    printf("The contents of file '%s' have been successfully copied into '%s' file.\n", input_file, output_file);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Insufficient parameters passed.\n");
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    copy_file(argv[1], argv[2]);

    return 0;
}
