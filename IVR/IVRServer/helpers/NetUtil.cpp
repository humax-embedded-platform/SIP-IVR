#include "NetUtil.h"

NetUtil::NetUtil() {}

bool NetUtil::ipv4ToSa(const char *ip_address, sockaddr_in &sa, int port)
{
    memset(&sa, 0, sizeof(sa));

    // Set the address family to IPv4
    sa.sin_family = AF_INET;

    // Convert IPv4 address from text to binary and store in sa.sin_addr
    if (inet_pton(AF_INET, ip_address, &(sa.sin_addr)) <= 0) {
        std::cerr << "Invalid IP address or inet_pton error." << std::endl;
        return false;
    }

    // Set the port number (convert from host byte order to network byte order)
    sa.sin_port = htons(port);
    return true;
}
