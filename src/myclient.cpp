/*
 * This C++ program serves as a client for a custom UDP protocol, designed to send data to a server and receive echoed responses. The main function initializes a socket and extracts server information from command line arguments, including the server's IP address, port number, Maximum Transmission Unit (MTU), input file path, and output file path. The client then establishes a connection with the server, performs file and directory checks, and opens the specified input file. The client processes the data in the input file by dividing it into packets, adding necessary overhead, and sending them to the server. The echoed packets received from the server are parsed, and their payloads are written to the output file. The program includes error handling mechanisms for various scenarios, such as incorrect command line arguments, socket failures, and file-related issues. Overall, this code represents a client implementation for a custom UDP protocol with error-checking capabilities.
 * */



#include <string.h>
#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <fstream>

using namespace std;

// Structure to hold server information
struct ServerInfo {
    string server_IP;
    int server_port;
    int mtu;
    string in_file_path;
    string out_file_path;
};

// Global variables
int overhead_length = 60;
int packet_counter = 0;
int bytes_read_from_echo = 0;
int bytes_read_from_in_file = 0;
int bytes_put_in_payload = 0;
int bytes_sent_to_server = 0;

// Function to handle errors and exit
void errorAndExit(string print_error) {
    int errnum;
    errnum = errno;

    if (errnum != 0) {
        cerr << print_error << "\n" << "errno: " << errnum << "\n";
    } else {
        cerr << print_error << "\n";
    }

    exit(EXIT_FAILURE);
}

// Function to parse command line arguments and populate ServerInfo structure
ServerInfo getCommandLineArgs(int argc, char** argv) {
    ServerInfo info;

    if (argc != 6) {
        cerr << "Incorrect number of command line arguments.\nFormat: ./myclient server_ip server_port mtu in_file_path out_file_path\n";
        exit(EXIT_FAILURE);
    }

    info.server_IP = argv[1];

    char* p;
    long converted_port = strtol(argv[2], &p, 10);
    if (*p) {
        errorAndExit("Please use a numerical port number");
    } else {
        info.server_port = converted_port;
    }

    if (info.server_port <= 1024 || info.server_port >= 65536) {
        errorAndExit("Port number should be greater than 1024 and less than 65536");
    }

    info.mtu = 0;
    if (isdigit(argv[3][0])) {
        info.mtu = stoi(argv[3]);
    } else {
        errorAndExit("Please enter a numerical mtu value");
    }

    struct stat buffer;
    int status;
    status = stat(argv[4], &buffer);
    if (status != 0) {
        errorAndExit("Problem getting status of file from in_file_path");
    } else {
        info.in_file_path = argv[4];
    }

    info.out_file_path = argv[5];
    ofstream file;

    int index_of_last_slash = 0;
    for (int i = 0; i < (int)strlen(info.out_file_path.c_str()); i++) {
        if (info.out_file_path.c_str()[i] == '/') {
            index_of_last_slash = i;
        }
    }

    if (index_of_last_slash == 0) {
        return info;
    }

    char directory_path[100];
    for (int i = 0; i < index_of_last_slash; i++) {
        directory_path[i] = info.out_file_path.c_str()[i];
    }

    int check;
    check = mkdir(directory_path, 0777);
    if (check < 0) {
        if (errno == EEXIST) {
            // cerr << "directory already exists\n";
        } else {
            errorAndExit("Unable to create directory at out_file_path");
        }
    }

    return info;
}

// Function to print server information
void printInfo(ServerInfo server_info1) {
    cout << "Server IP: " << server_info1.server_IP << "\n";
    cout << "Server Port: " << server_info1.server_port << "\n";
    cout << "MTU: " << server_info1.mtu << "\n";
    cout << "in file path: " << server_info1.in_file_path << "\n";
    cout << "out file path: " << server_info1.out_file_path << "\n";
}

// Function to send a packet to the server
void sendPacketToServer(int sockfd, const sockaddr *pservaddr, socklen_t servlen, string packet, int mtu, char* echoed_packet, int bytes_in_packet) {
    int n = 0;
    int s = 0;
    struct timeval tv;
    tv.tv_sec = 60; // 60s timeout
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); // Set timeout
    s = sendto(sockfd, packet.c_str(), bytes_in_packet, 0, pservaddr, servlen);
    if (s < 0) {
        errorAndExit("sendto() failed.");
    }
    n = recvfrom(sockfd, echoed_packet, mtu, 0, NULL, NULL);

    if (n < 0) {
        if (errno == EINTR) {
            recvfrom(sockfd, echoed_packet, mtu, 0, NULL, NULL);
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            errorAndExit("Cannot detect server");
        } else {
            errorAndExit("recvfrom() failed.");
        }
    }
}

// Function to parse echoed packet from the server
void parseEchoedPacket(char* echoed_packet, int mtu, FILE * outFile, int bytes_in_packet) {
    char payload[mtu];
    int packet_num = 0;

    bzero(&payload, sizeof(payload));

    sscanf(echoed_packet, "%*s %*s %d %*s %*s", &packet_num);

    if (packet_counter != packet_num) {
        errorAndExit("Packet loss detected");
    }

    int start_reading_flag = 0;
    int j = 0;
    for (int i = 0; i < bytes_in_packet; i++) {
        if (echoed_packet[i-1] == '\n' && echoed_packet[i-2] == ':') {
            start_reading_flag = 1;
        }
        if (start_reading_flag == 1) {
            payload[j] = echoed_packet[i];
            j++;
            bytes_read_from_echo++;
        }
    }

    fwrite(payload, sizeof(char), j, outFile);

    bzero(&echoed_packet, sizeof(echoed_packet));
    bzero(&payload, sizeof(payload));

    packet_counter++;
}

// Function to handle client-side processing
void doClientProcessing(int in_fd, FILE * outFile, int sockfd, const sockaddr *pservaddr, socklen_t servlen, int mtu) {
    if (overhead_length >= mtu) {
        errorAndExit("Required minimum MTU is 61");
    } else if (mtu >= 32000) {
        errorAndExit("MTU must be less than 32000");
    }

    int n;
    char data[mtu-overhead_length+1];
    char packet[mtu];
    int packet_num = 0;
    char echoed_packet[mtu];
    char first_part[overhead_length+1];
    int bytes_in_packet = 0;

    bzero(&first_part, sizeof(first_part));
    bzero(&echoed_packet, sizeof(echoed_packet));
    bzero(&data, sizeof(data));
    bzero(&packet, sizeof(packet));

    string packet1;

    while ((n = read(in_fd, data, mtu-overhead_length)) >= 0) {
        bytes_read_from_in_file += n;
        if (n < 0) {
            errorAndExit("Read() error");
        }

        if (n == 0) {
            break;
        }

        bytes_in_packet = sprintf(first_part, "\r\n\r\nPacket Num: %d\r\n\r\nLen: %d\r\n\r\nPayload:\n", packet_num, n);
        bytes_in_packet += n;
        packet1 = first_part;
        packet1.append(data, n);

        sendPacketToServer(sockfd, pservaddr, servlen, packet1, mtu, echoed_packet, bytes_in_packet);

        parseEchoedPacket(echoed_packet, mtu, outFile, bytes_in_packet);

        bytes_in_packet = 0;
        packet1.clear();
        bzero(&first_part, sizeof(first_part));
        bzero(&echoed_packet, sizeof(echoed_packet));
        bzero(&data, sizeof(data));
        bzero(&packet, sizeof(packet));
        packet_num++;
    }

    close(in_fd);
}

// Main function
int main(int argc, char** argv) {
    // Get command line arguments and populate ServerInfo structure
    ServerInfo info = getCommandLineArgs(argc, argv);

    // Initialize socket and server address structure
    int sockfd;
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(info.server_port);

    // Convert string IP address to binary equivalent
    if (inet_pton(AF_INET, info.server_IP.c_str(), &servaddr.sin_addr) <= 0) {
        errorAndExit("Error converting string IP to binary");
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct stat sb;

    // Get file status for input file
    if (stat(info.in_file_path.c_str(), &sb) == -1) {
        errorAndExit("stat() error");
    }

    // Open input file
    int in_fd;
    in_fd = open(info.in_file_path.c_str(), O_RDONLY);
    if (in_fd < 0) {
        errorAndExit("Error opening file at in_file_path");
    }

    // Open output file
    FILE * outFile;
    outFile = fopen(info.out_file_path.c_str(),"w");
    if (outFile == NULL) {
        fclose(outFile);
        errorAndExit("Error opening file at out_file_path");
    }

    // Perform client-side processing
    doClientProcessing(in_fd, outFile, sockfd, (sockaddr *) &servaddr, sizeof(servaddr), info.mtu);

    // Close files and socket
    fclose(outFile);
    close(in_fd);

    return 0;
}
