#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>  // for getaddrinfo()

#define PORT "69"             // TFTP uses UDP port 69
#define MAX_PACKET_SIZE 516   // 512 data bytes + 4 bytes header
#define BLOCK_SIZE 512        // Maximum size of data per block

// Simple error helper
void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Function to create a UDP socket and connect it to the server
int setup_socket(const char *server_name, struct addrinfo **server_info) {
    struct addrinfo hints, *res;
    int sockfd;
    // Hints tell getaddrinfo what we want
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    // Resolve the server name (hostname or IP)
    if (getaddrinfo(server_name, PORT, &hints, &res) != 0)
        error("Could not resolve server address");

    // Now create the socket
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        error("Failed to create socket");

    *server_info = res; // Pass back server info
    return sockfd;
}

// Build and send a Read Request (RRQ) packet
void send_rrq(int sockfd, const char *file_name, struct addrinfo *server_info) {
    char rrq_packet[MAX_PACKET_SIZE];
    size_t rrq_len;
    // Format: Opcode (2 bytes) | Filename | 0 | Mode ("octet") | 0
    rrq_packet[0] = 0x00; rrq_packet[1] = 0x01; // Opcode 01 = RRQ
    strcpy(rrq_packet + 2, file_name);         // Copy file name
    strcpy(rrq_packet + 2 + strlen(file_name) + 1, "octet"); // Add "octet" mode
    rrq_len = 2 + strlen(file_name) + 1 + strlen("octet") + 1;

    // Send it off to the server
    if (sendto(sockfd, rrq_packet, rrq_len, 0, server_info->ai_addr, server_info->ai_addrlen) < 0)
        error("Failed to send RRQ");

    printf("Sent RRQ for file \"%s\".\n", file_name);
}

// Receive DATA packets and send ACKs
void receive_file(int sockfd, const char *file_name, struct addrinfo *server_info) {
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);
    char buffer[MAX_PACKET_SIZE];
    char ack[4];
    int block_number = 1;  // Block numbers start from 1
    ssize_t bytes_received;
    // Open a file for writing
    FILE *file = fopen(file_name, "wb");
    if (!file) error("Failed to open file for writing");

    while (1) {
        // Wait for the DATA packet
        bytes_received = recvfrom(sockfd, buffer, MAX_PACKET_SIZE, 0,
                                  (struct sockaddr *)&from_addr, &addr_len);
        if (bytes_received < 4) error("Received invalid packet");
       int opcode = ntohs(*(short *)buffer);          // First 2 bytes: Opcode
        int received_block = ntohs(*(short *)(buffer + 2)); // Next 2 bytes: Block number

        if (opcode == 3 && received_block == block_number) {  // DATA packet
            // Write the data to the file
            fwrite(buffer + 4, 1, bytes_received - 4, file);
            printf("Received block %d, size %zd bytes. Sending ACK.\n", block_number, bytes_received - 4);
       // Prepare and send the ACK packet
            *(short *)ack = htons(4);              // Opcode 04 = ACK
            *(short *)(ack + 2) = htons(block_number);
            sendto(sockfd, ack, 4, 0, (struct sockaddr *)&from_addr, addr_len);

            block_number++; // Expect the next block now
            // If we got a short packet, it's the last block
            if (bytes_received < MAX_PACKET_SIZE) {
                printf("End of file received.\n");
                break;
            }
        } else if (opcode == 5) {  // ERROR packet
            fprintf(stderr, "Error from server: %s\n", buffer + 4);
            fclose(file);
            return;
        }
    }
    fclose(file);
    printf("File \"%s\" downloaded successfully.\n", file_name);
}

void getftp(const char *server_name, const char *file_name) {
    struct addrinfo *server_info;
    int sockfd;

    // 1. Set up the socket
    sockfd = setup_socket(server_name, &server_info);
    // 2. Send RRQ packet
    send_rrq(sockfd, file_name, server_info);

    // 3. Receive the file
    receive_file(sockfd, file_name, server_info);

    // 4. Cleanup
    close(sockfd);
    freeaddrinfo(server_info);
}
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_name_or_ip> <file_name>\n", argv[0]);
        return 1;
    }

    printf("Connecting to server: %s\n", argv[1]);
    printf("Requesting file: %s\n", argv[2]);
    getftp(argv[1], argv[2]);
    return 0;
}
