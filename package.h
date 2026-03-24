#ifndef PACKAGE_H
#define PACKAGE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations
typedef struct Package Package;
typedef struct FileMetadataPackage FileMetadataPackage;
typedef struct FileDataPackage FileDataPackage;
typedef struct TransferCompletePackage TransferCompletePackage;

// Enum first, because Package uses it
typedef enum {
    FILE_METADATA,
    FILE_DATA,
    TRANSFER_COMPLETE
} PackageType;

// Define structs that Package's union will contain
struct FileMetadataPackage {
    Package *data_packages;
    int file_size_bytes;
    int packet_size;
    char filename[256];
    char ip_address[16];
};

struct FileDataPackage {
    Package *next;
    int byte_offset;
    int data_length;
    char data[1024];
};

struct TransferCompletePackage {
    
};

// Now define Package, since all required types exist
struct Package {
    PackageType type;
    union {
        FileMetadataPackage file_metadata;
        FileDataPackage file_data;
        TransferCompletePackage transfer_complete;
    };
};

Package * create_package(const char *buffer, Package *previous_data_package);
void initialize_meta_package(Package *package, const char *buffer);
void initialize_data_package(Package *package, const char *buffer, Package *previous_package);
void initialize_transfer_complete_package(Package *package, const char *buffer, Package *previous_package);
void print_package(const Package *package);
void get_data_from_packages(const Package *header_package, char **file_data, size_t total_size);
void store_in_file(const Package *header_package);
int are_packages_valid(const Package *header_package);

#endif