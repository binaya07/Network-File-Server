#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define PORT 8080

int createSocket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

void setupServerAddress(struct sockaddr_in *servAddr) {
    servAddr->sin_family = AF_INET;
    servAddr->sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &servAddr->sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }
}

void connectToServer(int clientFd, struct sockaddr_in servAddr) {
    if (connect(clientFd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
}

void sendMessage(int clientFd, const char *message) {
    if (send(clientFd, message, strlen(message), 0) < 0) {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
}

void receiveMessage(int clientFd) {
    char buffer[1024] = { 0 };
    ssize_t valRead = read(clientFd, buffer, sizeof(buffer) - 1); // Subtract 1 for null terminator
    if (valRead < 0) {
        perror("Read failed");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", buffer);
}

void handleSignal(int sig) {
	exit(EXIT_FAILURE);
}

void trimNewline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}


void uploadFile(int clientFd, char *cmd) {
    char buffer[1024];
    char filePath[1024];
    strncpy(filePath, cmd + 3, sizeof(filePath) - 1);
    trimNewline(filePath);

    FILE *file = fopen(filePath, "rb");
    if (!file) {
        // Handle error
        printf("Error uploading file.");
        return;
    }
    // Send file name with cmd
    sendMessage(clientFd, cmd);
    // Send file data
    size_t bytesRead;
    bytesRead = fread(buffer, sizeof(char), sizeof(buffer), file);
    send(clientFd, buffer, bytesRead, 0);
    // Send end of file transfer indication
    fclose(file);
}

int main(int argc, char const *argv[]) {
    int clientFd;
    struct sockaddr_in servAddr;
	char message[1024];

	// Handler for process kill in terminal
    signal(SIGINT, handleSignal);

	// Socket setup
    clientFd = createSocket();
    if (clientFd < 0) {
        perror("Socket creation error");
        return -1;
    }

    setupServerAddress(&servAddr);
    connectToServer(clientFd, servAddr);

	// Get commands from terminal, send to server and print response
	while (1) {
        printf("Enter command: ");
        fgets(message, sizeof(message), stdin);
        if (strncmp(message, "up ", 3) == 0) {
            uploadFile(clientFd, message);
        } else {
            sendMessage(clientFd, message);
        }
        receiveMessage(clientFd);
    }

    close(clientFd);
    return 0;
}
