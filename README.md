# Simple Client-Server Application

This repository contains the implementation of a simple network file server using C and socket programming. The application demonstrates basic network communication between a client and a server in a Linux environment.

### Server and Client Functionality
   **Server**: Maintains a repository accessible to the client.
   
   **Client Commands**:
   ```
        ls - List subdirectories and files in the current directory.
        cd - Change the current working directory.
        cwd - Show the current working directory path.
        mkdir - Create a subdirectory in the current directory.
        rm - Delete a subdirectory or file in the current directory.
        up - Upload a file from the client to the current directory on the server.
        down - Download a file from the current directory on the server.
   ```

### Prerequisites
- C/C++ compiler (gcc or similar)

### Installation

1. **Clone the Repository**

   First, clone this repository to your local machine using Git:

   ```sh
   git clone [URL to this repo]
   ```

2. **Compile the Source Code**

   Navigate to the cloned directory, and compile both the server and client applications:

   ```sh
   gcc server.c -o server
   gcc client.c -o client
   ```

### Running the Application

1. **Start the Server**

   In one terminal window, start the server:

   ```sh
   ./server
   ```

2. **Run the Client**

   Open another terminal window and run the client:

   ```sh
   ./client
   ```

   The client will connect to the server and can send above listed commands to the server.
