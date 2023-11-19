# Simple Client-Server Application

This repository contains the implementation of a simple client-server application using C and socket programming. The application demonstrates basic network communication between a client and a server in a Linux environment.

## Getting Started

These instructions will guide you through setting up and running the project on your local machine for development and testing purposes.

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
   gcc server.cpp -o server
   gcc client.cpp -o client
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

   The client will connect to the server, send a message, and receive a response.

## Features

- **Server**: Listens for connections and responds to client requests.
- **Client**: Connects to the server, sends a message, and receives a response.
