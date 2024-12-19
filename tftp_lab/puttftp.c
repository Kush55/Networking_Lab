#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 69
#define MAX_PACKET_SIZE 516
#define BLOCK_SIZE 512

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void putftp(const char *server_ip, const char *file_name) {
    int sockfd;
    struct sockaddr_in server_addr, from_addr;
    socklen_t addr_len = sizeof(from_addr);
    char buffer[MAX_PACKET_SIZE], ack[4];
    int block_number = 0;
    ssize_t sent_bytes, received_bytes;
    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        error("Socket creation failed");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    // Prepare WRQ (Write Request) packet
    sprintf(buffer, "%c%s%c%s%c", 0x00, file_name, 0x00, "octet", 0x00);
    size_t len = 2 + strlen(file_name) + 1 + strlen("octet") + 1;

    // Send WRQ to server
    if (sendto(sockfd, buffer, len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error("Failed to send WRQ");

    FILE *file = fopen(file_name, "rb");
    if (!file)
        error("Failed to open file");

    while (1) {
        // Wait for ACK
        received_bytes = recvfrom(sockfd, ack, sizeof(ack), 0, (struct sockaddr *)&from_addr, &addr_len);
        if (received_bytes < 4 || ntohs(*(short *)ack) != 4 || ntohs(*(short *)(ack + 2)) != block_number) {
            fprintf(stderr, "ACK not received for block %d\n", block_number);
            fclose(file);
            close(sockfd);
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
}
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <file_name>\n", argv[0]);
        return 1;
    }
    putftp(argv[1], argv[2]);
    return 0;
}
