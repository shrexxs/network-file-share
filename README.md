# C++ Network File Server (Assignment 4)

This is a client-server application written in C++ that allows for file transfers over a network using TCP sockets.

## Features
* **Two-Way Transfers:** Supports both uploading files to the server and downloading files from the server.
* **File Listing:** The client can request and receive a list of all files in the server's directory.
* **Authentication:** A simple, hardcoded password is required for the client to connect.

## How to Run

1.  **Compile:**
    ```bash
    # Compile the server
    g++ -o server server.cpp -std=c++17

    # Compile the client
    g++ -o client client.cpp -std=c++17
    ```

2.  **Run:**
    * **Terminal 1:** Run the server
        ```bash
        ./server
        ```
    * **Terminal 2:** Run the client
        ```bash
        ./client
        ```