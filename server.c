#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <asm-generic/socket.h>

#define PORT 8080

void handleSignal(int sig) {
	exit(EXIT_FAILURE);
}

int createSocket() {
    int serverFd;
    int opt = 1;

    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("Setsockopt failed");
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    return serverFd;
}

void setupAddressStruct(struct sockaddr_in *address) {
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);
}

void bindSocket(int serverFd, struct sockaddr_in address) {
    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(serverFd);
        exit(EXIT_FAILURE);
    }
}

void startListening(int serverFd) {
    if (listen(serverFd, 3) < 0) {
        perror("Listen failed");
        close(serverFd);
        exit(EXIT_FAILURE);
    }
}

int acceptConnection(int serverFd, struct sockaddr_in address) {
    int clientSocket;
    socklen_t addrLen = sizeof(address);

    if ((clientSocket = accept(serverFd, (struct sockaddr *)&address, &addrLen)) < 0) {
        return -1;
    }

    return clientSocket;
}

void handleClientCommand(int clientSocket) {
    char buffer[1024] = {0};
    ssize_t valRead;
    char *helloMessage = "Hello from server";

    valRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (valRead < 0) {
        perror("Read failed");
        exit(EXIT_FAILURE);
    }

    buffer[valRead] = '\0';
    printf("Received command: %s", buffer);

    if (valRead < 0) {
        perror("Read failed");
        exit(EXIT_FAILURE);
    }

    if (send(clientSocket, helloMessage, strlen(helloMessage), 0) < 0) {
        perror("Send failed");
		exit(EXIT_FAILURE);
    } else {
        printf("Hello message sent\n");
    }
}

int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int serverFd, clientSocket;

	// Handler for process kill in terminal
    signal(SIGINT, handleSignal);

	// Create socket and listen to incoming connection requests
    serverFd = createSocket();
    setupAddressStruct(&address);
    bindSocket(serverFd, address);
    startListening(serverFd);
    
	clientSocket = acceptConnection(serverFd, address);

	if(clientSocket < 0) {
            perror("Accept failed - server shutting down");
    } else {
		while (1) {
        	handleClientCommand(clientSocket);	
		}
	}

    close(clientSocket);
    printf("Shutting down server\n");
    close(serverFd);
    return 0;
}
