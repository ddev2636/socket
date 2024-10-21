#include <iostream>
#include <fstream>
#include <string>
#include <cstring>   // For memset, strlen - used for handling strings
#include <unistd.h>  // For close() - used to close the socket
#include <arpa/inet.h>  // For sockaddr_in and inet_ntoa() - used for address manipulation

using namespace std;

#define PORT 8080  // The port on which the server listens for incoming data
#define BUFFER_SIZE 1024  // Size of the buffer for receiving data

class UDPServer {
private:
    int sockfd;  // Socket file descriptor, used to identify the socket
    struct sockaddr_in servaddr, cliaddr;  // Structures to hold server and client addresses
    socklen_t len;  // Length of the client address structure

public:
    UDPServer() {
        // Create a UDP socket using IPv4 (AF_INET) and UDP protocol (SOCK_DGRAM)
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("Socket creation failed");  // Output an error message if socket creation fails
            exit(EXIT_FAILURE);  // Exit if socket creation fails
        }
        cout << "[DEBUG SERVER] Socket created successfully." << endl;

        // Clear (zero out) the server and client address structures
        memset(&servaddr, 0, sizeof(servaddr));  // Reset server address structure
        memset(&cliaddr, 0, sizeof(cliaddr));    // Reset client address structure

        // Set server address information
        servaddr.sin_family = AF_INET;  // Use IPv4
        servaddr.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address
        servaddr.sin_port = htons(PORT);  // Set port number (converted to network byte order using htons)

        // Bind the socket to the server address and port
        if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            perror("Bind failed");  // Output an error if the bind fails
            close(sockfd);  // Close the socket before exiting
            exit(EXIT_FAILURE);  // Exit if the bind fails
        }
        cout << "[DEBUG SERVER] Bind successful on port " << PORT << "." << endl;

        len = sizeof(cliaddr);  // Set the length of the client address structure
    }

    void run() {
        char buffer[BUFFER_SIZE];  // Buffer for storing received data

        // Receive the file name from the client
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';  // Null-terminate the received string to handle it as a C-style string
        cout << "[DEBUG SERVER] Client requested file: " << buffer << endl;

        // Prepend "server/" to the filename to look for the file in the server directory
        string filename = "server/" + string(buffer);

        // Try to open the file requested by the client
        ifstream file(filename);
        if (!file) {
            string notFound = "NOTFOUND";  // Message to send if the file is not found
            sendto(sockfd, notFound.c_str(), notFound.size(), 0, (const struct sockaddr *)&cliaddr, len);
            cerr << "[DEBUG SERVER] Error: File not found - " << buffer << endl;
            close(sockfd);  // Close the socket before exiting
            exit(EXIT_FAILURE);  // Exit if file is not found
        }
        cout << "[DEBUG SERVER] File opened successfully: " << buffer << endl;

        // Send the first word as a greeting (could be any word or initial message)
        string word;
        file >> word;  // Read the first word from the file
        sendto(sockfd, word.c_str(), word.size(), 0, (const struct sockaddr *)&cliaddr, len);  // Send it to the client
        cout << "[DEBUG SERVER] Sent to client: " << word << endl;

        // Loop through the file and send each word until the end or "END" is found
        while (file >> word) {
            cout << "[DEBUG SERVER] Read word from file: " << word << endl;
            if (word == "END") {  // If the word is "END", signal the end of transmission
                sendto(sockfd, word.c_str(), word.size(), 0, (const struct sockaddr *)&cliaddr, len);
                cout << "[DEBUG SERVER] Sent to client: " << word << " (End of file)" << endl;
                break;  // Exit the loop when "END" is encountered
            }

            // Wait for the client request for the next word
            n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
            buffer[n] = '\0';  // Null-terminate the received string
            cout << "[DEBUG SERVER] Received from client: " << buffer << endl;

            // Send the current word to the client
            sendto(sockfd, word.c_str(), word.size(), 0, (const struct sockaddr *)&cliaddr, len);
            cout << "[DEBUG SERVER] Sent to client: " << word << endl;
        }

        cout << "[DEBUG SERVER] File transfer complete." << endl;
        file.close();  // Close the file after reading all words
        close(sockfd);  // Close the socket
        cout << "[DEBUG SERVER] Socket closed." << endl;
    }
};

int main() {
    UDPServer server;  // Create a UDPServer object
    server.run();  // Run the server (start listening for client requests)
    return 0;  // Exit the program successfully
}
