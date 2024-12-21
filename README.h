// README.h

/**
 * TFTP Client Application - README
 * 
 * DESCRIPTION:
 * This application is a Trivial File Transfer Protocol (TFTP) client
 * written in C, designed to connect to a TFTP server, request a file,
 * and download it with a specified block size. The program uses UDP
 * for communication and adheres to TFTP's RFC 1350 and RFC 2348 standards.
 *
 * FEATURES:
 * - Configurable block size for file transfer.
 * - Measures file download time.
 * - Handles standard TFTP error responses.
 * - Implements TFTP options such as "blksize".
 *
 * PREREQUISITES:
 * - A running TFTP server accessible via IP or hostname.
 * - The requested file must exist on the server.
 *
 * COMPILATION:
 * Use the following command to compile the program:
 * 
 * gcc -o tftp_client tftp_client.c
 * 
 * USAGE:
 * Run the program with the following syntax:
 * 
 * ./tftp_client <server_name_or_ip> <file_name> <blocksize>
 * 
 * EXAMPLES:
 * 1. Request a file named "example.txt" from server "192.168.1.1" with blocksize 512:
 *    ./tftp_client 192.168.1.1 example.txt 512
 * 
 * 2. Use a hostname instead of an IP address:
 *    ./tftp_client tftp.example.com data.bin 1024
 *
 * PARAMETERS:
 * - server_name_or_ip: The hostname or IP address of the TFTP server.
 * - file_name: The name of the file to download.
 * - blocksize: The block size to use for the TFTP transfer (must be between 8 and 65464).
 *
 * FUNCTIONAL DETAILS:
 * - The client sends a Read Request (RRQ) packet to the TFTP server.
 * - The RRQ includes an optional "blksize" parameter to specify the block size.
 * - The server responds with DATA packets or an ERROR packet.
 * - The client acknowledges each DATA packet with an ACK.
 * - The transfer ends when the client receives a DATA packet smaller than the block size.
 *
 * IMPORTANT NOTES:
 * - Ensure the TFTP server is reachable and properly configured.
 * - The client adheres to the "octet" transfer mode.
 * - Large block sizes may reduce overhead but require more memory.
 * 
 * ERROR HANDLING:
 * - Displays an error message and exits if:
 *   - Server address resolution fails.
 *   - The socket cannot be created.
 *   - RRQ transmission fails.
 *   - Invalid or error packets are received.
 * - Logs the error messages returned by the TFTP server.
 *
 * COPYRIGHT & LICENSE:
 * This software is provided "as-is" without any warranties.
 * Use it at your own risk for educational or personal purposes.
 * Redistribution and modification are allowed with proper attribution.
 */
