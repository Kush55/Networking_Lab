#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT "69"               // TFTP Port
#define MAX_PACKET_SIZE 516
#define BLOCK_SIZE 512

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int create_socket(const char *server_name, struct addrinfo **server_info) {
    int sockfd;
    struct addrinfo hints, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM;  // UDP
    if (getaddrinfo(server_name, PORT, &hints, server_info) != 0)
        error("Failed to resolve server address");

    for (p = *server_info; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) >= 0)
            return sockfd;  // Return reserved socket
    }

    error("Socket creation failed");
    return -1;
}
void putftp(const char *server_name, const char *file_name) {
    int sockfd;
    struct addrinfo *server_info;
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);
    char buffer[MAX_PACKET_SIZE], ack[4];
    int block_number = 0;
    ssize_t sent_bytes, received_bytes;

    sockfd = create_socket(server_name, &server_info);
    // Prepare WRQ (Write Request) packet
    sprintf(buffer, "%c%s%c%s%c", 0x00, file_name, 0x00, "octet", 0x00);
    size_t len = 2 + strlen(file_name) + 1 + strlen("octet") + 1;

    if (sendto(sockfd, buffer, len, 0, server_info->ai_addr, server_info->ai_addrlen) < 0)
        error("Failed to send WRQ");

    FILE *file = fopen(file_name, "rb");
    if (!file) error("Failed to open file");
    while (1) {
        received_bytes = recvfrom(sockfd, ack, sizeof(ack), 0, (struct sockaddr *)&from_addr, &addr_len);
        if (ntohs((short *)ack) != 4 || ntohs((short *)(ack + 2)) != block_number) {
            fprintf(stderr, "ACK error for block %d\n", block_number);
            break;
        }

        block_number++;
        buffer[0] = 0x00;
        buffer[1] = 0x03;  // DATA opcode
        *(short *)(buffer + 2) = htons(block_number);
        size_t data_len = fread(buffer + 4, 1, BLOCK_SIZE, file);
        sent_bytes = sendto(sockfd, buffer, data_len + 4, 0, (struct sockaddr *)&from_addr, addr_len);

        if (data_len < BLOCK_SIZE) break;
    }

    printf("File \"%s\" uploaded successfully.\n", file_name);
    fclose(file);
    close(sockfd);
    freeaddrinfo(server_info);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_name> <file_name>\n", argv[0]);
        return 1;
    }
    putftp(argv[1], argv[2]);
    return 0;
}
