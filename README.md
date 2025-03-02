# File Echo Application using UDP

This project implements a file echo application using UDP for reliable file transmission between a client and a server. The client sends a file in packets, and the server echoes the packets back to the client. The client then reassembles the file from the echoed packets. This project demonstrates UDP communication, file splitting, error handling, and packet reordering.

---

## File Structure

1. **src/**  
   Contains source files for the client and server.  
   - `myserver.cpp`  
   - `myclient.cpp`

2. **bin/**  
   Executable files generated by the Makefile.  
   - `myserver`  
   - `myclient`  
   - `myserver.o`  
   - `myclient.o`

3. **doc/**  
   Documentation of the application in plain text or PDF format.  
   - `README.pdf`

4. **Makefile**  
   Used to build the client program from the required source files.

---

## Documentation

### 1. Introduction
This application implements a client-server file echo system using UDP. The focus is on reliable packet communication, error handling, and file reconstruction.

### 2. Client Implementation
- Sends a file in mtu-byte sized packets to the server.
- Reconstructs the file from echoed packets.

### 3. Server Implementation
- Receives packets from the client and echoes them back.

### 4. Error Handling
- Detects packet loss, server unavailability, and handles socket API errors.

---

## Test Cases

- **Basic File Transmission**: Verify that the file is successfully sent, echoed, and reconstructed.
- **Packet Loss Detection**: Simulate packet loss and ensure the client exits with an error message.
- **Server Unavailability**: Ensure the client detects server unavailability.
- **Reordering of Echoed Packets**: Handle packet reordering and correctly reassemble the file.
- **Large File Transmission**: Test with files up to 1GB in size.
- **PDF/JPEG File Transmission**: Ensure that non-text files (PDF, JPEG) are transmitted and reconstructed correctly.

---

## Build Instructions

- Use the provided Makefile to build the executable files.  
- Executables are placed in the `bin/` directory.

---

## Usage

1. Run the server:  
   `./myserver <port>`

2. Run the client:  
   `./myclient <server_ip> <server_port> <mtu> <infile> <outfile>`

---

## Known Limitations
- Only supports basic file types and UDP-based transmission.
- No advanced error handling for network-related issues beyond basic packet loss detection.

---

## References
- "Unix Network Programming, Volume 1" by Richard Stevens  
- [GeeksforGeeks UDP Server-Client Example](https://www.geeksforgeeks.org/udp-server-client-implementation-c/)  
- [Stack Overflow UDP Programming](https://stackoverflow.com/questions/35568996/socket-programming-udp-client-server-in-c)  

---
