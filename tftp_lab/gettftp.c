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
void getftp(const char *server_name, const char *file_name) {
    int sockfd;
    struct addrinfo hints, *server_info, *p;
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);
    char buffer[MAX_PACKET_SIZE], ack[4];
    int block_number = 1;
    ssize_t received_bytes;
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


    // Prepare RRQ (Read Request) packet
    sprintf(buffer, "%c%s%c%s%c", 0x00, file_name, 0x00, "octet", 0x00);
    size_t len = 2 + strlen(file_name) + 1 + strlen("octet") + 1;

    // Send RRQ to server
    if (sendto(sockfd, buffer, len, 0, p->ai_addr, p->ai_addrlen) < 0)
        error("Failed to send RRQ");

    FILE *file = fopen(file_name, "wb");
    if (!file)
        error("Failed to create file");

    while (1) {
        // Receive DATA packet
        received_bytes = recvfrom(sockfd, buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *)&from_addr, &addr_len);
        if (received_bytes < 4)
            error("Packet too short");

        int opcode = ntohs(*(short *)buffer);
        int received_block = ntohs(*(short *)(buffer + 2));

        if (opcode == 3 && received_block == block_number) { // DATA packet
            fwrite(buffer + 4, 1, received_bytes - 4, file);

            // Send ACK
            *(short *)ack = htons(4);
            *(short *)(ack + 2) = htons(block_number);
            sendto(sockfd, ack, 4, 0, (struct sockaddr *)&from_addr, addr_len);
            block_number++;
            if (received_bytes < MAX_PACKET_SIZE) // End of file
                break;
        } else if (opcode == 5) { // ERROR packet
            fprintf(stderr, "Error received: %s\n", buffer + 4);
            fclose(file);
            close(sockfd);
            freeaddrinfo(server_info);
            exit(1);
        }
    }
    printf("File \"%s\" downloaded successfully.\n", file_name);
    fclose(file);
    close(sockfd);
    freeaddrinfo(server_info); // Free the server info
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_name> <file_name>\n", argv[0]);
        return 1;
    }
    getftp(argv[1], argv[2]);
    return 0;
}
