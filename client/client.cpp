#include <iostream>     // For standard input and output operations
#include <fstream>      // For file operations
#include <string>       // For using std::string
#include <cstring>      // For C-style string functions like memset, strlen
#include <unistd.h>     // For POSIX API functions like close()
#include <arpa/inet.h>  // For networking functions like inet_pton()

using namespace std;

// Definitions for port number, buffer size, and server IP address
#define PORT 8080          // Port number where the server is listening
#define BUFFER_SIZE 1024   // Size of the buffer for sending/receiving data
#define SERVER_IP "127.0.0.1" // IP address of the server (localhost here)

class UDPClient {
private:
    int sockfd;                  // Socket file descriptor, used for communication
    struct sockaddr_in servaddr; // Structure to hold server address details
    socklen_t len;               // Length of the server address structure

public:
    // Constructor: sets up the socket and server address information
    UDPClient() {
        // Create a socket using IPv4 (AF_INET), Datagram type (SOCK_DGRAM), and UDP protocol (0)
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {  // Check if socket creation failed
            perror("Socket creation failed");  // Print the error if any
            exit(EXIT_FAILURE);                // Exit the program if socket creation fails
        }
        cout << "[DEBUG CLIENT] Socket created successfully." << endl;

        // Clear the server address structure to initialize it
        memset(&servaddr, 0, sizeof(servaddr));

        // Set up the server address
        servaddr.sin_family = AF_INET;  // Use IPv4 addresses
        servaddr.sin_port = htons(PORT); // Convert port number to network byte order
        // Convert and set the server's IP address from text to binary form
        int res = inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);
        if (res <= 0) {  // Check if the address conversion failed
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }
        cout << "[DEBUG CLIENT] Server address set successfully." << endl;

        len = sizeof(servaddr);  // Set the size of the server address structure
    }

    // The main function to run the client program
    void run() {
        string filename;
        cout << "Enter the file name: ";
        cin >> filename;

        // Send the file name to the server using the socket
        // The sendto function sends data to the specified address (server)
        int bytesSent = sendto(sockfd, filename.c_str(), filename.size(), 0, 
                               (const struct sockaddr *)&servaddr, len);
        if (bytesSent < 0) {  // Check if the send operation failed
            perror("Failed to send filename");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        cout << "[DEBUG CLIENT] Filename sent to server: " << filename << endl;

        // Buffer to store incoming data from the server
        char buffer[BUFFER_SIZE];
        // Receive the server's response into the buffer
        int bytesReceived = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, 
                                     (struct sockaddr *)&servaddr, &len);
        if (bytesReceived < 0) {  // Check if the receive operation failed
            perror("Error receiving from server");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        buffer[bytesReceived] = '\0'; // Null-terminate the received data to form a proper string
        cout << "[DEBUG CLIENT] Received response from server: " << buffer << endl;

        // Check if the server responded with "NOTFOUND" indicating the file was not found
        if (strcmp(buffer, "NOTFOUND") == 0) {
            cout << "File not found on server." << endl;
            close(sockfd);  // Close the socket before exiting
            return;
        }

        // Open a file locally to store the received file data
        ofstream newFile("client/received_file.txt");
        if (!newFile) {  // Check if file creation/opening failed
            perror("Error creating file");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        cout << "[DEBUG CLIENT] Created 'received_file.txt' to store received data." << endl;

        int wordCount = 1;  // Variable to keep track of how many words have been received
        while (true) {
            // Prepare a request message for the next word
            string request = "WORD" + to_string(wordCount);
            bytesSent = sendto(sockfd, request.c_str(), request.size(), 0, 
                               (const struct sockaddr *)&servaddr, len);
            if (bytesSent < 0) {  // Check if the send operation failed
                perror("Error sending word request");
                break;  // Exit the loop if an error occurs
            }
            cout << "[DEBUG CLIENT] Sent word request: " << request << endl;

            // Receive the next word from the server
            bytesReceived = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, 
                                     (struct sockaddr *)&servaddr, &len);
            if (bytesReceived < 0) {  // Check if the receive operation failed
                perror("Error receiving word from server");
                break;  // Exit the loop if an error occurs
            }
            buffer[bytesReceived] = '\0'; // Null-terminate the received data
            cout << "[DEBUG CLIENT] Received word: " << buffer << endl;

            // Check if the received word is "END", indicating the end of file transfer
            if (strcmp(buffer, "END") == 0) {
                cout << "[DEBUG CLIENT] 'END' received. Stopping file transfer." << endl;
                break;
            }

            // Write the received word to the file and add a newline character
            newFile << buffer << "\n";
            cout << "[DEBUG CLIENT] Wrote word to file: " << buffer << endl;
            wordCount++;  // Increment the counter for the next word request
        }

        cout << "File transfer complete. Check 'received_file.txt'." << endl;
        newFile.close(); // Close the file after writing all data
        close(sockfd);   // Close the socket
    }
};

// Entry point of the program
int main() {
    UDPClient client;  // Create a UDPClient object
    client.run();      // Run the client logic
    return 0;          // Return 0 to indicate successful execution
}