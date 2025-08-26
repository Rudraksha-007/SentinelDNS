#define _WIN32_WINNT 0x0600
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include "dns_parser.h"
#include "logger.h"

#pragma comment(lib, "ws2_32.lib")

#define DNS_PORT 53
#define BUF_SIZE 512  // DNS packet max over UDP

struct DNSHeader {
    uint16_t id;       // Transaction ID
    uint16_t flags;    // qr opocode aa tc rd ra z rcode
    uint16_t qdcount;  //Number of questions
    uint16_t ancount;  
    uint16_t nscount;  
    uint16_t arcount;  
};

struct DNSRecord {
    std::string domain;
    struct in_addr ip;
};

bool ipStringToBytes(const char* ipStr, unsigned char out[4]) {
    unsigned int b1, b2, b3, b4;
    if (sscanf(ipStr, "%u.%u.%u.%u", &b1, &b2, &b3, &b4) != 4)
        return false;
    if (b1 > 255 || b2 > 255 || b3 > 255 || b4 > 255)
        return false;
    out[0] = (unsigned char)b1;
    out[1] = (unsigned char)b2;
    out[2] = (unsigned char)b3;
    out[3] = (unsigned char)b4;
    return true;
}

int main() {
    std::vector<DNSRecord> records = {
        {"example.com", {}},
        {"test.com", {}}
    };
    unsigned char ipBytes[4];
    // problem : Windows 11 KB5063878 is not allowing the server to bind to port 53 
    // wont let me compile the server 

    ipStringToBytes("93.184.216.34", ipBytes);
    records[0].ip.s_addr = *(uint32_t*)ipBytes; //network byte order conversion
    
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in serverAddr, clientAddr;

    char buffer[BUF_SIZE];

    // 1. Init Winsock library for the DNS server
    if (WSAStartup(MAKEWORD(2,2),&wsaData) != 0) {
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

    // 3. Bind the socket to port 53
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DNS_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed. pls run as Admin.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Authoritative DNS Server running on port 53...\n";

    // 4. Main loop
    while (true) {
        int clientAddrLen = sizeof(clientAddr);
        int bytesReceived = recvfrom(sock, buffer, BUF_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrLen);

        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recvfrom() failed\n";
            continue;
        }
        
        std::cout << "Got DNS query of size " << bytesReceived << " bytes\n";
        logMessage("Received DNS query of size " + std::to_string(bytesReceived) + " bytes");

        DNSHeader dns;
        memcpy(&dns, buffer, sizeof(DNSHeader));

        std::string domain = extractDomain(buffer);
        std::cout << "Domain: " << domain << "\n";
        logMessage("Extracted domain: " + domain);

        // TODO: Build response packet and send back
        char response[BUF_SIZE];
        int response_len = 0;

        // building the DNS response header:
        DNSHeader* response_header = (DNSHeader*)response;
        response_header->id = dns.id; // ID is already in network byte order from request
        response_header->flags = htons(0x8180); // Standard response flags
        response_header->qdcount = dns.qdcount; // Question count is same as request
        response_header->ancount = htons(0); // We'll add answers later
        response_header->nscount = htons(0);
        response_header->arcount = htons(0);
        response_len += sizeof(DNSHeader);
        
        // The question section from the query is copied directly to the response.
        char* question_start_in_query = buffer + sizeof(DNSHeader);
        int question_len = (char*)ptr - question_start_in_query;

        memcpy(response + response_len, question_start_in_query, question_len);
        response_len += question_len;

        // TODO: Build the answer section here
        bool found = false;
        char *answer_ptr = response + response_len;

        for (auto record : records) {
            if (domain == record.domain) {
                // 1. NAME: Add the 2-byte pointer (0xc00c)
                *(uint16_t*)answer_ptr = htons(0xc00c);
                answer_ptr += 2;

                // 2. TYPE: A Record (1)
                *(uint16_t*)answer_ptr = htons(1);
                answer_ptr += 2;

                // 3. CLASS: IN (1)
                *(uint16_t*)answer_ptr = htons(1);
                answer_ptr += 2;

                // 4. TTL: 3600 seconds
                *(uint32_t*)answer_ptr = htonl(3600);
                answer_ptr += 4;

                // 5. RDLENGTH: 4 bytes for an IPv4 address
                *(uint16_t*)answer_ptr = htons(4);
                answer_ptr += 2;

                // 6. RDATA: The IP address
                *(uint32_t*)answer_ptr = record.ip.s_addr;
                answer_ptr += 4;

                // Update total response length
                response_len = answer_ptr - response;

                // Update the answer count in the header
                response_header->ancount = htons(ntohs(response_header->ancount) + 1);
                
                found = true;
                break; // Found our record, stop searching
            }
        }

        // For now, just send back the header and question section
        sendto(sock, response, response_len, 0, (sockaddr*)&clientAddr, clientAddrLen);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}