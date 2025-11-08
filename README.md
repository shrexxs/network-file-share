# C++ Network File Server (Assignment 4)

This is a client-server application written in C++ that allows for file transfers over a network using TCP sockets.

## Features
* **Multi-Threaded Server:** Can handle multiple clients at the same time.
* **Two-Way Transfers:** Supports both uploading files to the server and downloading files from the server.
* **File Listing:** The client can request and receive a list of all files in the server's directory.
* **Authentication:** A simple, hardcoded password is required for the client to connect.

## How to Run

1.  **Compile:**
    ```bash
    # Compile the server
    # Note: The -pthread flag is now REQUIRED for multi-threading!
    g++ -o server server.cpp -std=c++17 -pthread

    # Compile the client (no change)
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
    * **Terminal 3 (Optional):** Run another `./client` at the same time to test the multi-threading.