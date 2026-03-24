// tcp_client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "package.h"
#include "file_manager.h"

#define PORT 8080
#define PACKET_SIZE 10
const char *ip_address = "10.30.8.146";



int get_response(int socket_fd, char *buffer, size_t buffer_capacity) {
    ssize_t bytes_received = recv(socket_fd, buffer, buffer_capacity - 1, 0);
    if (bytes_received < 0) {
        perror("Receive failed");
        return -1;
    }
    if (bytes_received == 0) {
        fprintf(stderr, "Server closed the connection\n");
        return -1;
    }
    buffer[bytes_received] = '\0'; // Ensure null-termination
    return bytes_received;
}

void send_metadata_packet(int file_size, int socket_fd, const char *filename, const char *ip_address) {
    char message[1024];
    sprintf(message, "{\"type\":\"meta\", \"file_size_bytes\": %d, \"packet_size\": %d, \"filename\": \"%s\", \"ip_address\": \"%s\"}",
            file_size, PACKET_SIZE, filename, ip_address);
    send(socket_fd, message, strlen(message), 0);
    get_response(socket_fd, message, 1024);
    if (strcmp(message, "Metadata received") != 0) {
        fprintf(stderr, "Unexpected response from server: '%s'\n", message);
        exit(EXIT_FAILURE);
    }
}

void send_data_packet(int byte_offset, const char *data, int data_length, int socket_fd) {
    char message[1024];
    sprintf(message, "{\"type\":\"data\", \"byte_offset\": %d, \"data_length\": %d, \"data\": \"%.*s\"}", byte_offset, data_length, data_length, data);
    send(socket_fd, message, strlen(message), 0);

    get_response(socket_fd, message, 1024);
    if (strcmp(message, "Data received") != 0) {
        fprintf(stderr, "Unexpected response from server: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

void send_transfer_complete_packet(int socket_fd) {
    char message[1024];
    sprintf(message, "{\"type\":\"complete\"}");
    send(socket_fd, message, strlen(message), 0);
    get_response(socket_fd, message, 1024);
    if (strcmp(message, "Transfer complete") != 0) {
        fprintf(stderr, "Unexpected response from server: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    ip_address = argc > 2 ? argv[2] : ip_address;
    int port = argc > 1 ? atoi(argv[1]) : PORT;
    // get the ip address of the computer running the client program and store it in the variable ip_address. If the user provides an ip address as a command line argument, use that instead of the default localhost ip address
    ip_address = argc > 2 ? argv[2] : ip_address;

    char * file_name = argc > 3 ? argv[3] : "example.txt";
    // get the filename of the file to be sent to the server from the command line arguments. If the user does not provide a filename, use "example.txt" as the default filename
    int socket_fd;
    struct sockaddr_in server_addr;
    char message[1024];
    char packet_buffer[1024];

    // 1. Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, ip_address, &server_addr.sin_addr);
    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Connected to server!\n");
    printf("Sending file and computer metadata to server...\n");

    int file_size = get_file_size(file_name);

    send_metadata_packet(file_size, socket_fd, file_name, ip_address);

    for (int i = 0; i < file_size; i += PACKET_SIZE) {
        send_data_packet(i, packet_buffer, get_bytes_from_file(file_name, i, packet_buffer, PACKET_SIZE), socket_fd);
    }

    send_transfer_complete_packet(socket_fd);

    printf("File transfer complete. Closing connection.\n");

    close(socket_fd);
    return 0;
}

