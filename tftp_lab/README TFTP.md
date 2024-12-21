
# TFTP Client Application

## Overview

This project provides a simple implementation of a TFTP (Trivial File Transfer Protocol) client, capable of both uploading (`puttftp.c`) and downloading files (`gettftp.c`) from a TFTP server. It supports specifying a custom block size for data transmission.

## Features

- **File Upload (PUT):** Uploads a file to the TFTP server.
- **File Download (GET):** Downloads a file from the TFTP server.
- **Block Size Negotiation:** Supports custom block sizes for efficient data transfer.
- **Timing Analysis:** Measures the time taken for file transfers.

## Requirements

- A TFTP server running and accessible from the client machine.
- A C compiler (e.g., `gcc`) to compile the source code.
- Network connectivity to the server.

## Compilation

To compile the TFTP client programs:

```bash
gcc -o gettftp gettftp.c
gcc -o puttftp puttftp.c
```

## Usage

### File Download (GET)

Command:
```bash
./gettftp <server_name_or_ip> <file_name> <blocksize>
```

Example:
```bash
./gettftp 192.168.1.100 example.txt 1024
```

- `<server_name_or_ip>`: IP address or hostname of the TFTP server.
- `<file_name>`: Name of the file to download.
- `<blocksize>`: Block size for the transfer (must be between 8 and 65464).

### File Upload (PUT)

Command:
```bash
./puttftp <server_name_or_ip> <file_name> <blocksize>
```

Example:
```bash
./puttftp 192.168.1.100 upload.txt 512
```

- `<server_name_or_ip>`: IP address or hostname of the TFTP server.
- `<file_name>`: Name of the file to upload.
- `<blocksize>`: Block size for the transfer (must be between 8 and 65464).

## Code Highlights

### Common Functions

- **Error Handling:** `exit_with_error(const char *msg)` - Handles and displays errors, then exits the program.
- **Socket Setup:** `setup_udp_socket(const char *server_name, struct addrinfo **server_info)` - Configures and establishes a UDP socket.

### `gettftp.c`

- **RRQ Packet Formation:** Sends a Read Request (RRQ) with block size negotiation.
- **File Reception:** Handles data packets, acknowledges them, and writes the received data to a file.

### `puttftp.c`

- **WRQ Packet Formation:** Sends a Write Request (WRQ) with block size negotiation.
- **File Transmission:** Reads file data, sends data packets, and waits for acknowledgments.

## Notes

1. Ensure the TFTP server supports block size negotiation (`blksize` option).
2. Make sure the file permissions and paths are correct for both the client and server.

## License

This project is distributed under the MIT License. Feel free to use and modify it.

## Authors

- **[Your Name]** - Developer
