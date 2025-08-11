#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

#define DNS_PORT 53
#define BUF_SIZE 512  // DNS packet max over UDP

int main() {
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in serverAddr, clientAddr;
    char buffer[BUF_SIZE];

    // 1. Init Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // 2. Create socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    // 3. Bind to port 53
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DNS_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed. Try running as Admin.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Authoritative DNS Server running on port 53...\n";

    // 4. Main loop
    while (true) {
        int clientAddrLen = sizeof(clientAddr);
        int bytesReceived = recvfrom(sock, buffer, BUF_SIZE, 0,
                                     (sockaddr*)&clientAddr, &clientAddrLen);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recvfrom() failed\n";
            continue;
        }

        std::cout << "Got DNS query of size " << bytesReceived << " bytes\n";

        // TODO: Parse domain from buffer here (RFC1035)
        // TODO: Build response packet and send back

        // For now just echo back the same bytes (not valid DNS!)
        sendto(sock, buffer, bytesReceived, 0,
               (sockaddr*)&clientAddr, clientAddrLen);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}