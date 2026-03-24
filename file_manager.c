#include <stdio.h>
#include <stdlib.h>
#include "file_manager.h"

int get_file_size(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("File opening failed");
        return -1;
    }
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fclose(file);
    return size;
}

int get_bytes_from_file(const char *filename, int starting_byte, char *buffer, size_t buffer_capacity) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("File opening failed");
        return -1;
    }
    fseek(file, starting_byte, SEEK_SET);
    size_t bytes_read = fread(buffer, 1, buffer_capacity, file);
    fclose(file);
    return bytes_read;
}

void create_file(const char *filename, const char *data, size_t data_length) {
    char * dir_begin = "received_files/";
    char full_path[512];

    snprintf(full_path, sizeof(full_path), "%s%s", dir_begin, filename);
    FILE *file = fopen(full_path, "wb");
    if (file == NULL) {
        printf("Failed to create file at path: %s\n", full_path);
        perror("File creation failed");
    }
    size_t bytes_written = fwrite(data, 1, data_length, file);
    fclose(file);
}