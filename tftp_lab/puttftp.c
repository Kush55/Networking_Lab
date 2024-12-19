#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT "69"             // TFTP port number
#define MAX_PACKET_SIZE 516   // 4-byte header + 512 bytes of data
#define BLOCK_SIZE 512        // Data size per packet
void exit_with_error(const char *msg) {
    perror(msg);
    exit(1);
}

int create_udp_socket(const char *server_address, struct addrinfo **server_info) {
    struct addrinfo hints, *res;
    int udp_socket;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM;  // UDP socket
    // Resolve the server address
    if (getaddrinfo(server_address, PORT, &hints, &res) != 0)
        exit_with_error("Failed to resolve server address");

    // Create a socket
    if ((udp_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        exit_with_error("Socket creation failed");

    *server_info = res; // Pass server info back to the caller
    return udp_socket;
}

void send_write_request(int udp_socket, const char *file_name, struct addrinfo *server_info) {
    char wrq_packet[MAX_PACKET_SIZE];
    size_t packet_size;

    // Format WRQ packet: Opcode (2 bytes) | Filename | 0 | Mode ("octet") | 0
    wrq_packet[0] = 0x00;
    wrq_packet[1] = 0x02;  // WRQ Opcode
    strcpy(wrq_packet + 2, file_name);
    strcpy(wrq_packet + 2 + strlen(file_name) + 1, "octet");
    packet_size = 2 + strlen(file_name) + 1 + strlen("octet") + 1;
    // Send the WRQ packet to the server
    if (sendto(udp_socket, wrq_packet, packet_size, 0, server_info->ai_addr, server_info->ai_addrlen) < 0)
        exit_with_error("Failed to send WRQ");

    printf("Write request sent for file \"%s\".\n", file_name);
}
int wait_for_ack(int udp_socket, int expected_block) {
    char ack_packet[4];
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);
    ssize_t received_bytes;

    // Wait for an ACK packet from the server
    received_bytes = recvfrom(udp_socket, ack_packet, sizeof(ack_packet), 0, 
                              (struct sockaddr *)&from_addr, &addr_len);
    if (received_bytes < 4)
        exit_with_error("Invalid ACK packet received");

    int opcode = ntohs(*(short *)ack_packet);
    int block_number = ntohs(*(short *)(ack_packet + 2));

    // Verify it's an ACK for the correct block
    if (opcode != 4 || block_number != expected_block) {
        fprintf(stderr, "Unexpected ACK: Block %d (Expected %d)\n", block_number, expected_block);
        exit(1);
    }

    return block_number;
}

void upload_file(int udp_socket, const char *file_name, struct addrinfo *server_info) {
    FILE *file;
    char data_packet[MAX_PACKET_SIZE];
    int block_number = 0;
    size_t bytes_read;

    file = fopen(file_name, "rb");
    if (!file) exit_with_error("Failed to open file for reading");
    while (1) {
        // Prepare the DATA packet
        data_packet[0] = 0x00;
        data_packet[1] = 0x03;  // DATA Opcode
        block_number++;
        *(short *)(data_packet + 2) = htons(block_number);  // Block number

        // Read up to 512 bytes from the file
        bytes_read = fread(data_packet + 4, 1, BLOCK_SIZE, file);
        // Send the DATA packet
        if (sendto(udp_socket, data_packet, bytes_read + 4, 0, 
                   server_info->ai_addr, server_info->ai_addrlen) < 0)
            exit_with_error("Failed to send DATA packet");

        printf("Sent block %d, size %zu bytes.\n", block_number, bytes_read);
        // Wait for an ACK for this block
        wait_for_ack(udp_socket, block_number);
        printf("Received ACK for block %d.\n", block_number);

        // If the last block was smaller than 512 bytes, we're done
        if (bytes_read < BLOCK_SIZE) {
            printf("Upload complete. All blocks sent.\n");
            break;
        }
    }

    fclose(file);
}

void put_file_to_server(const char *server_name, const char *file_name) {
    struct addrinfo *server_info;
    int udp_socket;

    // Create the UDP socket and connect to the server
    udp_socket = create_udp_socket(server_name, &server_info);

    // Send the WRQ to start the upload
    send_write_request(udp_socket, file_name, server_info);

    // Upload the file block by block
    upload_file(udp_socket, file_name, server_info);

    // Clean up
    close(udp_socket);
    freeaddrinfo(server_info);
}
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_name_or_ip> <file_name>\n", argv[0]);
        return 1;
    }

    printf("Connecting to server: %s\n", argv[1]);
    printf("Uploading file: %s\n", argv[2]);
    put_file_to_server(argv[1], argv[2]);

    return 0;
}
