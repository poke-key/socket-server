#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h>

#pragma comment(lib, "Ws2_32.lib")
#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_PASSWORD_LENGTH 100

int check_password_strength(const char* password) {
    int score = 0;
    int length = strlen(password);
    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;

    if (length >= 8) {
        score++;
    }

    for (int i = 0; i < length; i++) {
        if (isupper(password[i])) has_upper = 1;
        else if (islower(password[i])) has_lower = 1;
        else if (isdigit(password[i])) has_digit = 1;
        else has_special = 1;
    }

    score += has_upper + has_lower + has_digit + has_special;

    return (score << 4) | (has_upper << 3) | (has_lower << 2) | (has_digit << 1) | has_special;
}

void get_password_feedback(int result, int length, char* feedback, int feedback_size) {
    int score = result >> 4;
    int has_upper = (result & 8) != 0;
    int has_lower = (result & 4) != 0;
    int has_digit = (result & 2) != 0;
    int has_special = (result & 1) != 0;

    snprintf(feedback, feedback_size, "Password strength: ");
    if (score < 3) strcat(feedback, "Weak\n");
    else if (score < 5) strcat(feedback, "Moderate\n");
    else strcat(feedback, "Strong\n");

    strcat(feedback, "\nSuggestions for improvement:\n");
    if (length < 8) strcat(feedback, "- Password is too short. It should be at least 8 characters.\n");
    if (!has_upper) strcat(feedback, "- Add uppercase letters for stronger password.\n");
    if (!has_lower) strcat(feedback, "- Add lowercase letters for stronger password.\n");
    if (!has_digit) strcat(feedback, "- Add numbers for stronger password.\n");
    if (!has_special) strcat(feedback, "- Add special characters for stronger password.\n");
    if (score == 5) strcat(feedback, "Great job! Your password is strong.\n");
}

int main() {
    WSADATA wsaData;
    SOCKET server_fd = INVALID_SOCKET;
    SOCKET client_socket = INVALID_SOCKET;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    printf("Creating socket...\n");
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    printf("Binding socket...\n");
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Listening for connections...\n");
    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Server is running on port %d. Waiting for connections...\n", PORT);

    while(1) {
        printf("Waiting to accept a client connection...\n");
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            continue;
        }

        printf("Connection accepted. Waiting for password...\n");
        int result = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (result > 0) {
            printf("Received password: %s\n", buffer);
            
            // Check password strength
            int score = check_password_strength(buffer);
            char feedback[512] = {0};
            get_password_feedback(score, strlen(buffer), feedback, sizeof(feedback));
            
            // Send feedback to client
            if (send(client_socket, feedback, (int)strlen(feedback), 0) == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
            } else {
                printf("Feedback sent to client\n");
            }
        } else if (result == 0) {
            printf("Connection closing...\n");
        } else {
            printf("recv failed with error: %d\n", WSAGetLastError());
        }
        closesocket(client_socket);
    }

    // Cleanup
    closesocket(server_fd);
    WSACleanup();
    return 0;
}