#ifndef DNS_PARSER_H
#define DNS_PARSER_H

#include <string>

std::string extractDomain(const unsigned char* buffer);

#endif // DNS_PARSER_H