#ifndef NETUTIL_H
#define NETUTIL_H

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

class NetUtil
{
    NetUtil();
public:
    static bool ipv4ToSa(const char *ip_address, struct sockaddr_in &sa, int port);
};

#endif // NETUTIL_H
