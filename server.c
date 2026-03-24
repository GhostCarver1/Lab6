// tcp_server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "package.h"
#include "file_manager.h"

#define PORT 8080
#define BUFFER_SIZE 1024

void send_response(int client_socket, PackageType package_type) {
    char response[BUFFER_SIZE];
    if (package_type == FILE_METADATA) {
        sprintf(response, "Metadata received");
    } else if (package_type == FILE_DATA) {
        sprintf(response, "Data received");
    } else if (package_type == TRANSFER_COMPLETE) {
        sprintf(response, "Transfer complete");
    } else {
        sprintf(response, "Unknown package type received");
    }
    send(client_socket, response, strlen(response), 0);
}

// this doesnt need to be a double pointer, remove them

void advance_package(Package **header_package, Package **incoming_package, Package **last_package) {
    if ((*incoming_package)->type == FILE_METADATA) {
        *header_package = *incoming_package;
        *last_package = *incoming_package;
    } else if ((*incoming_package)->type == FILE_DATA) {  
        (*last_package)->file_data.next = *incoming_package;
        *last_package = *incoming_package;
    } else if ((*incoming_package)->type == TRANSFER_COMPLETE) {
        (*last_package)->file_data.next = *incoming_package;
        *last_package = *incoming_package;
    } else {
        fprintf(stderr, "Unknown package type received\n");
    }
}

int main() {

    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 3. Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // 4. Listen for incoming connections

    printf("Server listening on port %d...\n", PORT);


    while (1) {

        char message[1024];
        char response[1024];

        client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // if the package is a header file, 

        Package * header_package = NULL;
        Package * last_package = NULL;

        while (1) {
            int bytes_received = recv(client_socket, message, BUFFER_SIZE-1, 0);
            if (bytes_received == 0) {
                printf("Client disconnected\n");
                break;
            } else if (bytes_received < 0) {
                perror("Receive failed");
                break;
            }
            message[bytes_received] = '\0'; // Ensure null-termination
            Package * incoming_package = create_package(message, last_package);
            if (incoming_package == NULL) {
                fprintf(stderr, "Failed to create package from received message\n");
                break;
            }
            send_response(client_socket, incoming_package->type);
            advance_package(&header_package, &incoming_package, &last_package);   
        }
        close (client_socket);

        if (!are_packages_valid(header_package)) {
            fprintf(stderr, "Received an invalid package from client, causing a fault\n");
        }
        else 
        {
            printf("Received valid package from client, storing in file...\n");
            printf("received_files/%s\n", header_package->file_metadata.filename);
            store_in_file(header_package);
        }

        Package *current = header_package;
        while (current != NULL) {
            Package *next = current->file_data.next;
            free(current);
            current = next;
        }
    }
    close(server_fd);   
    return 0;
}
