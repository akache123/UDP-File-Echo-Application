/*
 * This C++ program represents a simple UDP server that echoes back any data it receives from clients. The server listens on a specified port, which is obtained as a command line argument, and continuously receives UDP datagrams from clients. Upon receiving a datagram, the server sends it back to the originating client. The main function initializes the server socket, binds it to a specific port, and then enters a perpetual loop where it calls the handleServerProcessing function. The handleServerProcessing function receives and sends UDP datagrams, ensuring a continuous back-and-forth communication with clients. The program includes error handling mechanisms for socket creation, binding, and data transfer. Additionally, it defines a maximum buffer length for the UDP messages. Overall, this code serves as a foundational structure for a basic UDP echo server.
 * */



#include <string.h>
#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>

using namespace std;

#define MAX_BUFFER_LENGTH 32000 // Maximum MTU for the client

// Function to handle server-side processing
void handleServerProcessing(int serverSocket, sockaddr *clientAddress, socklen_t clientAddressLength) {
    int receivedBytes;
    int sentBytes;
    socklen_t length;
    char message[MAX_BUFFER_LENGTH];

    string packet;

    for (;;) {
        length = clientAddressLength;
        receivedBytes = recvfrom(serverSocket, message, MAX_BUFFER_LENGTH, 0, clientAddress, &length); // Read datagram
        if (receivedBytes < 0) {
            cerr << "recvfrom() failed. Exiting now.\n";
            exit(EXIT_FAILURE);
        }
        sentBytes = sendto(serverSocket, message, receivedBytes, 0, clientAddress, length); // Send it back to the sender
        if (sentBytes < 0) {
            cerr << "sendto() failed. Exiting now.\n";
            exit(EXIT_FAILURE);
        }
        // cout << message;
    }
}

// Function to get port number from command line arguments
uint16_t getPort(int argumentCount, char** argumentValues) {
    int portNumber = 0;

    // Check for the correct number of command line arguments
    if (argumentCount > 2) {
        cerr << "Too many command line arguments.\nFormat: ./myserver port_number\n";
        exit(EXIT_FAILURE);
    }

    string portNumberString = argumentValues[1];

    // Check if the port number is an integer
    if (isdigit(portNumberString[0])) {
        portNumber = stoi(portNumberString);
    } else {
        cerr << "Please use a correct port number. Exiting now.\n";
        exit(EXIT_FAILURE);
    }

    // Check if the port number is within the valid range
    if (portNumber <= 1024 || portNumber >= 65536) {
        cerr << "Port number should be greater than 1024 and less than 65536\n";
        exit(EXIT_FAILURE);
    }

    return portNumber;
}

int main(int argumentCount, char** argumentValues) {
    // Get the port number from command line arguments
    uint16_t portNumber = getPort(argumentCount, argumentValues);

    // Initialize socket and address structures
    int serverSocket;
    struct sockaddr_in serverAddress, clientAddress;

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0); // Create a UDP socket using SOCK_DGRAM
    if (serverSocket < 0) {
        cerr << "socket() failed. Exiting now.\n";
        exit(EXIT_FAILURE);
    }
    bzero(&serverAddress, sizeof(serverAddress)); // Zero out server address

    // Set up server struct info
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(portNumber); // Converts from MY local byte order to network standard

    // Bind the socket
    if (bind(serverSocket, (sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "bind() failed. Exiting now.\n";
        exit(EXIT_FAILURE);
    }

    // Start server processing
    handleServerProcessing(serverSocket, (sockaddr *) &clientAddress, sizeof(clientAddress));

    return 0;
}
