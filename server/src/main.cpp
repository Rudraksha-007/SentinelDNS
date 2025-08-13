#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

#define DNS_PORT 53
#define BUF_SIZE 512  // DNS packet max over UDP

struct DNSHeader {
    uint16_t id;       // Transaction ID
    uint16_t flags;    // QR, Opcode, AA, TC, RD, RA, Z, RCODE
    uint16_t qdcount;  // Number of questions
    uint16_t ancount;  // Number of answer RRs
    uint16_t nscount;  // Number of authority RRs
    uint16_t arcount;  // Number of additional RRs
};


int main() {
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in serverAddr, clientAddr;

    char buffer[BUF_SIZE];

    // 1. Init Winsock library for the DNS server
    if (WSAStartup(MAKEWORD(2,2),&wsaData) != 0) {
        // if the version is not 2.2, we can't proceed
        std::cerr << "WSAStartup failed\n";
        return 1;
    }


    // 2. Create socket
    // AF_INET is for making sure IPv4 is used
    // SOCK_DGRAM is for socket type= UDP
    // IPPROTO_UDP specifies the UDP protocol
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    // 3. Bind the socket to port 53 setup:

    // tells the OS that this socket will use IPv4 internet addresses 
    // address family field of the sockaddr_in struct 
    // set it to AF_INET ie address family : internet (IPv4)
    serverAddr.sin_family = AF_INET;

    // sin_port is the port number we want to listen on 
    // htons converts the port number from host byte order to network byte order
    // its called Host to Network Short
    //I want to store the port number my socket will bind to in a struct (sockaddr_in).
    // Network protocols require multi-byte numbers like ports to be in big-endian (network byte order).
    // My CPU might store numbers differently, so I call htons() to convert my port from my CPU’s format to less-endian(wtv the CPU supports) before taking it from windows.
    // This way, it works on any CPU

    serverAddr.sin_port = htons(DNS_PORT);

    // tells the API to setup the socket to listen on all available IP addresses:
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
        int bytesReceived = recvfrom(sock, buffer, BUF_SIZE, 0,(sockaddr*)&clientAddr, &clientAddrLen);

        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recvfrom() failed\n";
            continue;
        }
        
        std::cout << "Got DNS query of size " << bytesReceived << " bytes\n";
        // might break the terminal:
        // std::cout<<"Buffer content: "<<std::endl;
        // for(auto ch:buffer){
        //     std::cout<<(int)ch<<" ";
        // }
        // std::cout<<std::endl;

        // TODO: Parse domain from buffer here (RFC1035)
        // parsing header : 
        DNSHeader* dns = (DNSHeader*)buffer;
        uint16_t qdcount = ntohs(dns->qdcount); 

        // this ptr is pointing inside the buffer itself exactly after the DNSHeader(skipping initial 12bytes)
        unsigned char* ptr = (unsigned char*)(buffer + sizeof(DNSHeader));

        std::string domain;
        while (*ptr != 0) {
            int label_len = *ptr;
            ptr++;
            for (int i = 0; i < label_len; i++) {
                domain.push_back(*ptr);
                ptr++;
            }
            domain.push_back('.'); // separate labels with dot
        }
        if (!domain.empty() && domain.back() == '.') {
            domain.pop_back();
        }
        ptr++; // skip the null byte

        // Extract QTYPE and QCLASS
        // std::cout<<(uint16_t)*ptr<<std::endl;
        // note : the ptr is pointing to the byte after the domain name
        // QTYPE and QCLASS are 2 bytes each the ptr points to each char which is 1byte 
        // we declare a uint16_t variable to hold the QTYPE and QCLASS values
        // and use memcpy to copy the 2 bytes from the ptr to the variable
        // the pointer itself points to a location in the buffer that is 1 byte long 
        // but memcpy takes the size of qtype which is 2 bytes
        // so it copies the next 2 bytes from the ptr to the qtype variable

        uint16_t qtype;
        memcpy(&qtype, ptr, sizeof qtype);   // copies 2 bytes
        qtype = ntohs(qtype);
        ptr += 2;

        uint16_t qclass;
        memcpy(&qclass, ptr, sizeof qclass);
        qclass = ntohs(qclass);
        ptr += 2;

        std::cout << "Domain: " << domain << "\n";
        std::cout << "QTYPE: " << qtype << " QCLASS: " << qclass << "\n";
        
        // TODO: Build response packet and send back
        
        // For now just echo back the same bytes (not valid DNS!)
        sendto(sock, buffer, bytesReceived, 0,
               (sockaddr*)&clientAddr, clientAddrLen);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}