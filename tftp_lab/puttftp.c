#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h> // For getaddrinfo

#define PORT "69"
#define MAX_PACKET_SIZE 516
#define BLOCK_SIZE 512
void error(const char *msg) {
    perror(msg);
    exit(1);
}

void putftp(const char *server_name, const char *file_name) {
    int sockfd;
    struct addrinfo hints, *server_info, *p;
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);
    char buffer[MAX_PACKET_SIZE], ack[4];
    int block_number = 0;
    ssize_t sent_bytes, received_bytes;

    // Use getaddrinfo to resolve server address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM;  // UDP

    if (getaddrinfo(server_name, PORT, &hints, &server_info) != 0)
        error("Failed to resolve server address");

    // Create UDP socket
    for (p = server_info; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) >= 0)
            break;
    }
    if (p == NULL)
        error("Socket creation failed");

    // Prepare WRQ (Write Request) packet
    sprintf(buffer, "%c%s%c%s%c", 0x00, file_name, 0x00, "octet", 0x00);
    size_t len = 2 + strlen(file_name) + 1 + strlen("octet") + 1;
    // Send WRQ to server
    if (sendto(sockfd, buffer, len, 0, p->ai_addr, p->ai_addrlen) < 0)
        error("Failed to send WRQ");

    FILE *file = fopen(file_name, "rb");
    if (!file)
        error("Failed to open file");

    while (1) {
        // Wait for ACK
        received_bytes = recvfrom(sockfd, ack, sizeof(ack), 0, (struct sockaddr *)&from_addr, &addr_len);
        if (received_bytes < 4 || ntohs((short *)ack) != 4 || ntohs((short *)(ack + 2)) != block_number) {
            fprintf(stderr, "ACK not received for block %d\n", block_number);
            fclose(file);
            close(sockfd);
            freeaddrinfo(server_info);
            exit(1);
        }

        block_number++;

        // Prepare DATA packet
        buffer[0] = 0x00;
        buffer[1] = 0x03; // DATA opcode
        *(short *)(buffer + 2) = htons(block_number);

        size_t data_len = fread(buffer + 4, 1, BLOCK_SIZE, file);
        sent_bytes = sendto(sockfd, buffer, data_len + 4, 0, (struct sockaddr *)&from_addr, addr_len);
        if (sent_bytes < 0)
            error("Failed to send DATA packet");

        if (data_len < BLOCK_SIZE) // End of file
            break;
    }

    printf("File \"%s\" uploaded successfully.\n", file_name);
    fclose(file);
    close(sockfd);
    freeaddrinfo(server_info);
}
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_name_or_ip> <file_name>\n", argv[0]);
        return 1;
    }
    putftp(argv[1], argv[2]);
    return 0;
}
