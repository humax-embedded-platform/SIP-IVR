#ifndef MEDIASERVER_H
#define MEDIASERVER_H

#include "Defines.h"
#include <thread>
#include <memory>
class RequestHandler;

class MediaServer
{
public:
    MediaServer(int port = DEFAULT_PORT);

    void startServer();
    void handleConnection(int clientFd);

    // void parse
private:
    int _serverFd;
    int _serverPort;
    std::shared_ptr<std::thread> _serverThread;
    std::shared_ptr<RequestHandler> _requestHandler;
};

#endif // MEDIASERVER_H
