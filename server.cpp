#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <filesystem>
#include <sstream>
#include <fstream>      

namespace fs = std::filesystem;

#define PORT 8080
#define BUFFER_SIZE 1024 
#define CORRECT_PASSWORD "shreyaCrypt" 

//Helper function to handle download request 
void handle_download(int new_socket) {
    
    char buffer[BUFFER_SIZE] = {0};

    std::stringstream file_list_stream;
    try {
        for (const auto & entry : fs::directory_iterator(fs::current_path())) {
            file_list_stream << entry.path().filename().string() << "\n";
        }
    } catch (fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
    std::string file_list = file_list_stream.str();
    if (file_list.empty()) file_list = "No files in directory.\n";

    send(new_socket, file_list.c_str(), file_list.length(), 0);
    std::cout << "File list sent to client." << std::endl;
    
    memset(buffer, 0, sizeof(buffer)); 
    read(new_socket, buffer, 1024);
    std::string requested_file = std::string(buffer);
    std::cout << "Client wants to download: " << requested_file << std::endl;

    std::ifstream file(requested_file, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: File not found - " << requested_file << std::endl;
        const char *errmsg = "ERROR: File not found";
        send(new_socket, errmsg, strlen(errmsg), 0);
    } else {
        const char *okmsg = "FILE_OK";
        send(new_socket, okmsg, strlen(okmsg), 0);
        
        std::cout << "Sending file: " << requested_file << std::endl;
        
        while (file.read(buffer, BUFFER_SIZE)) {
            send(new_socket, buffer, file.gcount(), 0);
        }
        if (file.gcount() > 0) {
            send(new_socket, buffer, file.gcount(), 0);
        }
        
        file.close();
        std::cout << "File sent successfully." << std::endl;
    }
}

// Helper function to handle upload request 
void handle_upload(int new_socket) {
    
    char buffer[BUFFER_SIZE] = {0};

    memset(buffer, 0, sizeof(buffer));
    read(new_socket, buffer, 1024);
    std::string filename = std::string(buffer);

    std::string new_filename = "uploaded_" + filename;
    std::cout << "Client wants to upload: " << filename << std::endl;
    std::cout << "Saving as: " << new_filename << std::endl;

    const char *okmsg = "OK_TO_SEND";
    send(new_socket, okmsg, strlen(okmsg), 0);

    std::ofstream output_file(new_filename, std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not create local file " << new_filename << std::endl;
        return;
    }

    int bytes_received = 0;
    while ((bytes_received = read(new_socket, buffer, BUFFER_SIZE)) > 0) {
        output_file.write(buffer, bytes_received);
    }
    
    output_file.close();
    std::cout << "File upload complete." << std::endl;
}


int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0}; 

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }
        std::cout << "Client connected! Waiting for password..." << std::endl;

        // Read the password from the client
        memset(buffer, 0, sizeof(buffer));
        read(new_socket, buffer, 1024);

        if (strcmp(buffer, CORRECT_PASSWORD) != 0) {
            
            std::cout << "Authentication failed. Disconnecting client." << std::endl;
            send(new_socket, "AUTH_FAILED: Wrong password", 26, 0);
            close(new_socket);
            continue; // Go back to accept()
        }

        std::cout << "Authentication successful!" << std::endl;
        send(new_socket, "AUTH_OK", 7, 0);

        
        memset(buffer, 0, sizeof(buffer));
        read(new_socket, buffer, 1024);
        
        if (strcmp(buffer, "DOWNLOAD") == 0) {
            std::cout << "Received DOWNLOAD command." << std::endl;
            handle_download(new_socket);
        } else if (strcmp(buffer, "UPLOAD") == 0) {
            std::cout << "Received UPLOAD command." << std::endl;
            handle_upload(new_socket);
        } else {
            std::cerr << "Unknown command: " << buffer << std::endl;
        }

        
        std::cout << "Client disconnected." << std::endl;
        close(new_socket); 
    } 
    
    close(server_fd); 
    return 0;
}