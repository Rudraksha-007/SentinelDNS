#include "dns_parser.h"
#include <string>

std::string extractDomain(const unsigned char *buffer)
{
    std::string domain;
    // set pointer to skip the dns header 
    const unsigned char *ptr = buffer + sizeof(DNSHeader); 
    

    while (*ptr != 0)
    {
        int label_len = *ptr;
        ptr++;
        for (int i = 0; i < label_len; i++)
        {
            domain.push_back(*ptr);
            ptr++;
        }
        domain.push_back('.'); 
    }
    if (!domain.empty() && domain.back() == '.')
    {
        domain.pop_back(); // Remove the trailing dot
    }
    ptr++; //skip nullbytre

    return domain;
}