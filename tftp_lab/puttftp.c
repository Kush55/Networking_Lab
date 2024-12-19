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
#define MAX_PACKET_SIZE 65536   // Max packet size (RFC 2348 limit)

// Utility function for error handling
void exit_with_error(const char *msg) {
    perror(msg);
    exit(1);
}

// Create a UDP socket and resolve server address
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
// Send WRQ packet with blocksize option
void send_wrq_with_blocksize(int udp_socket, const char *file_name, int blocksize, struct addrinfo *server_info) {
    char wrq_packet[MAX_PACKET_SIZE];
    size_t wrq_len;

    // Format WRQ: Opcode(2 bytes) | Filename | 0 | Mode ("octet") | 0 | "blksize" | 0 | "blocksize value" | 0
    wrq_packet[0] = 0x00; wrq_packet[1] = 0x02; // WRQ opcode
    int offset = 2;
    strcpy(wrq_packet + offset, file_name); offset += strlen(file_name) + 1;
    strcpy(wrq_packet + offset, "octet"); offset += strlen("octet") + 1;

    // Add blocksize option
    strcpy(wrq_packet + offset, "blksize"); offset += strlen("blksize") + 1;
    sprintf(wrq_packet + offset, "%d", blocksize); offset += strlen(wrq_packet + offset) + 1;

    wrq_len = offset;

    // Send WRQ packet to the server
    if (sendto(udp_socket, wrq_packet, wrq_len, 0, server_info->ai_addr, server_info->ai_addrlen) < 0)
        exit_with_error("Failed to send WRQ with blocksize");

    printf("Sent WRQ for file \"%s\" with blocksize=%d.\n", file_name, blocksize);
}

// Send file with the negotiated blocksize
void upload_file_with_blocksize(int udp_socket, const char *file_name, int blocksize, struct addrinfo *server_info) {
    FILE *file;
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);
    char data_packet[MAX_PACKET_SIZE];
    char ack_packet[4];
    int block_number = 0;
    ssize_t bytes_read, bytes_received;
    file = fopen(file_name, "rb");
    if (!file) exit_with_error("Failed to open file for reading");

    while (1) {
        // Prepare DATA packet
        data_packet[0] = 0x00; data_packet[1] = 0x03; // DATA opcode
        block_number++;
        *(short *)(data_packet + 2) = htons(block_number);
        // Read up to blocksize bytes from the file
        bytes_read = fread(data_packet + 4, 1, blocksize, file);

        // Send the DATA packet
        if (sendto(udp_socket, data_packet, bytes_read + 4, 0, server_info->ai_addr, server_info->ai_addrlen) < 0)
            exit_with_error("Failed to send DATA packet");

        printf("Sent block %d, size %zd bytes.\n", block_number, bytes_read);

        // Wait for ACK
        bytes_received = recvfrom(udp_socket, ack_packet, sizeof(ack_packet), 0, 
                                  (struct sockaddr *)&from_addr, &addr_len);
        if (bytes_received < 4)
            exit_with_error("Invalid ACK packet");

        int opcode = ntohs(*(short *)ack_packet);
        int ack_block = ntohs(*(short *)(ack_packet + 2));

        if (opcode != 4 || ack_block != block_number) {
            fprintf(stderr, "Unexpected ACK: Block %d (Expected %d)\n", ack_block, block_number);
            break;
        }

        printf("Received ACK for block %d.\n", block_number);

        // Stop if the last block (less than blocksize) has been sent
        if (bytes_read < blocksize) {
            printf("Last block sent. Upload complete.\n");
            break;
        }
    }
    fclose(file);
}

// Measure time for file upload
void putftp_with_timing(const char *server_name, const char *file_name, int blocksize) {
    struct timeval start, end;
    double elapsed_time;
    struct addrinfo *server_info;
    int udp_socket;
    // Setup socket
    udp_socket = setup_udp_socket(server_name, &server_info);

    gettimeofday(&start, NULL); // Start the timer

    // Send WRQ and upload the file
    send_wrq_with_blocksize(udp_socket, file_name, blocksize, server_info);
    upload_file_with_blocksize(udp_socket, file_name, blocksize, server_info);
    gettimeofday(&end, NULL); // Stop the timer

    // Calculate elapsed time
    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Upload completed with blocksize %d in %.3f seconds.\n", blocksize, elapsed_time);

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
    printf("Uploading file: %s with blocksize: %d\n", file_name, blocksize);
    putftp_with_timing(server_name, file_name, blocksize);

    return 0;
}
