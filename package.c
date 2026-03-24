#include "package.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_manager.h"

Package * create_package (const char *buffer, Package * previous_package) {
    Package *package = malloc(sizeof(Package));
    if (package == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    char type_str[16];
    sscanf(buffer, "{\"type\": \"%15[^\"]\"}", type_str);
    if (strcmp(type_str, "meta") == 0) {
        initialize_meta_package(package, buffer);
    } else if (strcmp(type_str, "data") == 0) {
        initialize_data_package(package, buffer, previous_package);
    } else if (strcmp(type_str, "complete") == 0) {
        initialize_transfer_complete_package(package, buffer, previous_package);
    } else {
        fprintf(stderr, "Unknown message type: %s\n", type_str);
        exit(EXIT_FAILURE);
    }
    return package;
}

int are_packages_valid(const Package *header_package) {
    if (header_package==NULL)
    {
        return 0;
    }
    Package *current = (Package *)header_package;
    while (current != NULL) {
        if (current->type == FILE_METADATA) {
            if (current->file_metadata.file_size_bytes < 0 || current->file_metadata.packet_size <= 0) {
                return 0;
            }
        } else if (current->type == FILE_DATA) {
            if (current->file_data.data_length < 0 || current->file_data.data_length > sizeof(current->file_data.data)) {
                return 0;
            }
        } else if (current->type == TRANSFER_COMPLETE) {
            // No specific validation for transfer complete package
        } else {
            return 0; // Unknown package type
        }
        current = current->file_data.next;
    }
    return 1;
}

void initialize_meta_package(Package *package, const char *buffer) {
    sscanf(buffer, "{\"type\": \"meta\", \"file_size_bytes\": %d, \"packet_size\": %d, \"filename\": \"%255[^\"]\", \"ip_address\": \"%15[^\"]\"}",
           &package->file_metadata.file_size_bytes,
           &package->file_metadata.packet_size,
           package->file_metadata.filename,
           package->file_metadata.ip_address);
    package->type = FILE_METADATA; // Default type, will be overwritten
    package->file_metadata.data_packages = NULL;
    package->file_data.next = NULL;
}

void initialize_data_package(Package *package, const char *buffer, Package *previous_package) {
    sscanf(buffer, "{\"type\": \"data\", \"byte_offset\": %d, \"data_length\": %d, \"data\": \"%1023[^\"]\"}",
            &package->file_data.byte_offset,
            &package->file_data.data_length,
            package->file_data.data);
    package->type = FILE_DATA; // Default type, will be overwritten
    package->file_data.next = NULL;
    if (previous_package != NULL) {
        previous_package->file_data.next = package;
    }
}

void initialize_transfer_complete_package(Package *package, const char *buffer, Package *previous_package) {
    package->type = TRANSFER_COMPLETE; // Default type, will be overwritten
    if (previous_package != NULL) {
        previous_package->file_data.next = package;
    }
}

void print_package(const Package *package) {
    while (1)
    {
        if (package->type == FILE_METADATA) {
            printf("Metadata package: file size = %d, packet size = %d, filename = %s \n {",
                   package->file_metadata.file_size_bytes,
                   package->file_metadata.packet_size,
                   package->file_metadata.filename);
        } else if (package->type == FILE_DATA) {
            for (int i = 0; i < package->file_data.data_length; i++) {
                char c = package->file_data.data[i];
                if (c == '\n') {
                    printf("\\n");
                } else if (c == '\r') {
                    printf("\\r");
                } else if (c == '\t') {
                    printf("\\t");
                } else if (c == '\\') {
                    printf("\\\\");
                } else if (c == '\"') {
                    printf("\\\"");
                } else if (c < 32 || c > 126) {
                    printf("\\x%02x", (unsigned char)c);
                } else {
                putchar(package->file_data.data[i]);
                }
            }
            //printf("%s", package->file_data.data);
        }
        if (package->type == TRANSFER_COMPLETE) {
            printf(" }\n");
            break;
        }
        package = package->file_data.next;
    }

}

void store_in_file(const Package *header_package) {
    char *file_data;
    get_data_from_packages(header_package, &file_data, header_package->file_metadata.file_size_bytes);
    char * file_path = malloc(512);
    if (file_path == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    create_file(header_package->file_metadata.filename, file_data, header_package->file_metadata.file_size_bytes);
    free(file_data);
}

void get_data_from_packages(const Package *package, char **buffer, size_t data_size){
    char *data_buffer = malloc(data_size + 1);
    if (data_buffer == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    size_t bytes_copied = 0;
    while (package != NULL) {
        if (package->type == FILE_DATA) {
            size_t bytes_to_copy = package->file_data.data_length;
            if (bytes_copied + bytes_to_copy > data_size) {
                bytes_to_copy = data_size - bytes_copied;
            }
            memcpy(data_buffer + bytes_copied, package->file_data.data, bytes_to_copy);
            bytes_copied += bytes_to_copy;
        }
        package = package->file_data.next;
    }
    data_buffer[bytes_copied] = '\0';
    *buffer = data_buffer;
}
