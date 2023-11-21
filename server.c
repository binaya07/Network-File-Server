#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <asm-generic/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

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

void trimNewline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

void createServerHomeDirectory() {
    struct stat st = {0};

    if (stat("server_home", &st) == -1) {
        mkdir("server_home", 0700);
    }
}

void handleClientCommand(int clientSocket) {
    char buffer[1024] = {0};
    ssize_t valRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (valRead < 0) {
        perror("Read failed");
        exit(EXIT_FAILURE);
    }

    buffer[valRead] = '\0';
    printf("Received command: %s", buffer);
    trimNewline(buffer);

    if (strcmp(buffer, "cwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            send(clientSocket, cwd, strlen(cwd), 0);
        } else {
            perror("getcwd() error");
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(buffer, "ls") == 0) {
        printf("1");
        // FILE *fp;
        // char path[1035];
        // printf("1");
        // fp = popen("ls server_home", "r");
        // printf("2");
        // if (fp == NULL) {
        //     perror("Failed to run command");
        //     exit(EXIT_FAILURE);
        // }

        // // Read the output a line at a time - send it to the client
        // while (fgets(path, sizeof(path), fp) != NULL) {
        //     printf("3");
        //     send(clientSocket, path, strlen(path), 0);
        // }

        // pclose(fp);
    } else {
        // Other command handling
        perror("Send failed");
		exit(EXIT_FAILURE);
    }

    // if (send(clientSocket, helloMessage, strlen(helloMessage), 0) < 0) {
    //     perror("Send failed");
	// 	exit(EXIT_FAILURE);
    // } else {
    //     printf("Hello message sent\n");
    // }
}

int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int serverFd, clientSocket;

    // Creates a server_home directory
    createServerHomeDirectory();

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
		// Listen to client commands and handle them
		while (1) {
        	handleClientCommand(clientSocket);	
		}
	}

    close(clientSocket);
    printf("Shutting down server\n");
    close(serverFd);
    return 0;
}
