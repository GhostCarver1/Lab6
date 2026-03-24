#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H
#include <stdio.h>
#include <stdlib.h>

int get_file_size(const char *filename);
int get_bytes_from_file(const char *filename, int starting_byte, char *buffer, size_t buffer_capacity);
void create_file(const char *filename, const char *data, size_t data_length);


#endif 