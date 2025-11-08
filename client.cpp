#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <fstream>      

#define PORT 8080
#define BUFFER_SIZE 1024
#define PASSWORD "shreyaCrypt" // The password we'll send

// --- Helper function to handle download ---
void handle_download(int sock, const std::string& filename) {
    // (Code from Day 4 - No changes needed)
    // ... (rest of the function is identical to previous step) ...
    std::cout << "Requested file: " << filename << std::endl;
    char buffer[BUFFER_SIZE] = {0};

    memset(buffer, 0, sizeof(buffer));
    int bytes_received = read(sock, buffer, 1024);
    
    if (bytes_received > 0 && strcmp(buffer, "FILE_OK") == 0) {
        std::string new_filename = "downloaded_" + filename;
        std::ofstream output_file(new_filename, std::ios::binary);
        if (!output_file.is_open()) {
            std::cerr << "Error: Could not create local file " << new_filename << std::endl;
        } else {
            std::cout << "Receiving file... saving as " << new_filename << std::endl;
            while ((bytes_received = read(sock, buffer, BUFFER_SIZE)) > 0) {
                output_file.write(buffer, bytes_received);
            }
            output_file.close();
            std::cout << "File download complete." << std::endl;
        }
    } else {
        std::cerr << "Server error: " << buffer << std::endl;
    }
}

// --- Helper function to handle upload ---
void handle_upload(int sock, const std::string& filename) {
    // (Code from Day 4 - No changes needed)
    // ... (rest of the function is identical to previous step) ...
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open local file " << filename << std::endl;
        return;
    }

    send(sock, filename.c_str(), filename.length(), 0);
    char buffer[BUFFER_SIZE] = {0};
    read(sock, buffer, 1024);

    if (strcmp(buffer, "OK_TO_SEND") != 0) {
        std::cerr << "Server is not ready to receive file." << std::endl;
        return;
    }

    std::cout << "Uploading file: " << filename << std::endl;
    while (file.read(buffer, BUFFER_SIZE)) {
        if (send(sock, buffer, file.gcount(), 0) == -1) {
            std::cerr << "Error sending file chunk" << std::endl;
            break;
        }
    }
    if (file.gcount() > 0) {
        send(sock, buffer, file.gcount(), 0);
    }
    file.close();
    std::cout << "File upload complete." << std::endl;
}


int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[4096] = {0}; 

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    std::cout << "Connected to server!" << std::endl;

    // --- NEW DAY 5 LOGIC ---
    
    // 1. Send the password immediately
    std::cout << "Sending password..." << std::endl;
    send(sock, PASSWORD, strlen(PASSWORD), 0);

    // 2. Wait for authentication reply
    memset(buffer, 0, sizeof(buffer));
    read(sock, buffer, 1024);

    if (strcmp(buffer, "AUTH_OK") != 0) {
        std::cerr << "Authentication failed: " << buffer << std::endl;
        close(sock);
        return -1;
    }
    
    std::cout << "Authentication successful!" << std::endl;

    // --- END NEW DAY 5 LOGIC ---


    // --- DAY 4 LOGIC (Now runs *after* auth) ---
    std::string command;
    std::cout << "Enter command (upload / download): ";
    std::getline(std::cin, command);

    if (command == "download") {
        send(sock, "DOWNLOAD", 8, 0); 
        memset(buffer, 0, sizeof(buffer));
        read(sock, buffer, 4096);
        std::cout << "\n--- Files on Server ---" << std::endl;
        std::cout << buffer; 
        std::cout << "-------------------------" << std::endl;

        std::string filename;
        std::cout << "Which file do you want to download? ";
        std::getline(std::cin, filename); 
        send(sock, filename.c_str(), filename.length(), 0);

        handle_download(sock, filename);

    } else if (command == "upload") {
        send(sock, "UPLOAD", 6, 0); 
        std::string filename;
        std::cout << "Which local file do you want to upload? ";
        std::getline(std::cin, filename);
        handle_upload(sock, filename);
    } else {
        std::cerr << "Invalid command." << std::endl;
    }
    // --- END DAY 4 LOGIC ---

    close(sock);
    return 0;
}