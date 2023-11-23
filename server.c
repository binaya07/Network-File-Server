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
    if (chdir("server_home") != 0) {
        perror("Failed to change directory to server_home");
        exit(EXIT_FAILURE);
    }
}

void handleCwd(int clientSocket) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        send(clientSocket, cwd, strlen(cwd), 0);
    } else {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }
}

void handleLs(int clientSocket) {
    FILE *fp;
    char path[1035];
    int count = 0;
    fp = popen("ls", "r");
    if (fp == NULL) {
        perror("Failed to run command");
        exit(EXIT_FAILURE);
    }
    // Read the output a line at a time - send it to the client
    while (fgets(path, sizeof(path), fp) != NULL) {
        send(clientSocket, path, strlen(path), 0);
        count++;
    }
    if (count == 0) {
        const char *noContentMsg = "No Contents";
        send(clientSocket, noContentMsg, strlen(noContentMsg), 0);
    }
    pclose(fp);
}

void handleCd(int clientSocket, char* argument) {
    if (chdir(argument) != 0) {
        // Send error message back to client
        const char *errorMsg = "Error changing directory";
        send(clientSocket, errorMsg, strlen(errorMsg), 0);
    } else {
        handleCwd(clientSocket);
    }
}

void handleMkdir(int clientSocket, char* argument) {
    struct stat st = {0};
    const char *msg;

    if (stat(argument, &st) == -1) {
        if (mkdir(argument, 0700) == 0) {
            msg = "Created directory";
        } else {
            msg = "Failed creating directory.";
        }
    } else {
        msg = "Directory exists";
    } 
    send(clientSocket, msg, strlen(msg), 0);
}

void handleRm(int clientSocket, char* argument) {
    struct stat st;
    const char *msg; 
    if (stat(argument, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            // It's a directory
            if (rmdir(argument) == 0) {
                msg = "Directory removed successfully";
            } else {
                msg = "Error removing directory";
            }
        } else {
            // It's a file
            if (remove(argument) == 0) {
                msg = "File removed successfully";
            } else {
                msg = "Error removing file";
            }
        }
    } else {
        msg = "File or directory does not exist";
    }
    send(clientSocket, msg, strlen(msg), 0);
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

    char *command = strtok(buffer, " ");
    char *argument = strtok(NULL, " ");

    if (strcmp(buffer, "cwd") == 0) {
        handleCwd(clientSocket);
    } else if (strcmp(buffer, "ls") == 0) {
        handleLs(clientSocket);
    } else if (strcmp(command, "cd") == 0) {
        handleCd(clientSocket, argument);
    } else if (strcmp(command, "mkdir") == 0) {
        handleMkdir(clientSocket, argument);
    } else if (strcmp(command, "rm") == 0) {
        handleRm(clientSocket, argument);
    } else {
        char* unknownCmdMsg = "Unknown command";
        send(clientSocket, unknownCmdMsg, strlen(unknownCmdMsg), 0);
    }
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
