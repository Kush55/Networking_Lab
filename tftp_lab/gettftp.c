#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h> // For gettimeofday()

#define PORT "69"               // TFTP port number
#define DEFAULT_BLOCK_SIZE 512  // Default block size
#define MAX_PACKET_SIZE 65536   // Max packet size for RFC 2348

void exit_with_error(const char *msg) {
    perror(msg);
    exit(1);
}

int setup_udp_socket(const char *server_name, struct addrinfo **server_info) {
    struct addrinfo hints, *res;
    int udp_socket;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if (getaddrinfo(server_name, PORT, &hints, &res) != 0)
        exit_with_error("Failed to resolve server address");

    udp_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (udp_socket < 0)
        exit_with_error("Socket creation failed");

    *server_info = res;
    return udp_socket;
}
void send_rrq_with_blocksize(int udp_socket, const char *file_name, int blocksize, struct addrinfo *server_info) {
    char rrq_packet[MAX_PACKET_SIZE];
    size_t rrq_len;

    // Format RRQ: Opcode(2 bytes) | Filename | 0 | Mode ("octet") | 0 | "blksize" | 0 | "blocksize value" | 0
    rrq_packet[0] = 0x00; rrq_packet[1] = 0x01; // RRQ opcode
    int offset = 2;
    strcpy(rrq_packet + offset, file_name); offset += strlen(file_name) + 1;
    strcpy(rrq_packet + offset, "octet"); offset += strlen("octet") + 1;

    // Add the "blksize" option
    strcpy(rrq_packet + offset, "blksize"); offset += strlen("blksize") + 1;
    sprintf(rrq_packet + offset, "%d", blocksize); offset += strlen(rrq_packet + offset) + 1;

    rrq_len = offset;
    // Send the RRQ packet
    if (sendto(udp_socket, rrq_packet, rrq_len, 0, server_info->ai_addr, server_info->ai_addrlen) < 0)
        exit_with_error("Failed to send RRQ with blocksize");

    printf("Sent RRQ for file \"%s\" with blocksize=%d.\n", file_name, blocksize);
}
void receive_file_with_blocksize(int udp_socket, const char *file_name, int blocksize) {
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);
    char buffer[MAX_PACKET_SIZE];
    char ack[4];
    int block_number = 1;
    ssize_t bytes_received;

    FILE *file = fopen(file_name, "wb");
    if (!file) exit_with_error("Failed to open file for writing");
    while (1) {
        // Receive DATA packet
        bytes_received = recvfrom(udp_socket, buffer, blocksize + 4, 0, (struct sockaddr *)&from_addr, &addr_len);
        if (bytes_received < 4)
            exit_with_error("Received invalid DATA packet");

        int opcode = ntohs(*(short *)buffer);
        int received_block = ntohs(*(short *)(buffer + 2));
        if (opcode == 3 && received_block == block_number) { // DATA packet
            // Write the received data to the file
            fwrite(buffer + 4, 1, bytes_received - 4, file);
            printf("Received block %d, size %zd bytes. Sending ACK.\n", block_number, bytes_received - 4);

            // Send ACK
            *(short *)ack = htons(4); // ACK opcode
            *(short *)(ack + 2) = htons(block_number);
            sendto(udp_socket, ack, 4, 0, (struct sockaddr *)&from_addr, addr_len);

            block_number++;
            // Stop when the last packet (less than blocksize) is received
            if (bytes_received - 4 < blocksize) {
                printf("End of file received.\n");
                break;
            }
        } else if (opcode == 5) { // ERROR packet
            fprintf(stderr, "Error received from server: %s\n", buffer + 4);
            fclose(file);
            return;
        }
    }
    fclose(file);
    printf("File \"%s\" downloaded successfully.\n", file_name);
}

void gettftp_with_timing(const char *server_name, const char *file_name, int blocksize) {
    struct timeval start, end;
    double elapsed_time;
    struct addrinfo *server_info;
    int udp_socket;
    // Setup the socket
    udp_socket = setup_udp_socket(server_name, &server_info);

    gettimeofday(&start, NULL); // Start the timer

    // Send RRQ and download the file
    send_rrq_with_blocksize(udp_socket, file_name, blocksize, server_info);
    receive_file_with_blocksize(udp_socket, file_name, blocksize);
   gettimeofday(&end, NULL); // Stop the timer

    // Calculate elapsed time
    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Download completed with blocksize %d in %.3f seconds.\n", blocksize, elapsed_time);

    // Cleanup
    close(udp_socket);
    freeaddrinfo(server_info);
}
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <server_name_or_ip> <file_name> <blocksize>\n", argv[0]);
        return 1;
    }

    const char *server_name = argv[1];
    const char *file_name = argv[2];
    int blocksize = atoi(argv[3]);

    if (blocksize < 8 || blocksize > 65464) {
        fprintf(stderr, "Blocksize must be between 8 and 65464.\n");
        return 1;
    }
    printf("Connecting to server: %s\n", server_name);
    printf("Requesting file: %s with blocksize: %d\n", file_name, blocksize);
    gettftp_with_timing(server_name, file_name, blocksize);

    return 0;
}
