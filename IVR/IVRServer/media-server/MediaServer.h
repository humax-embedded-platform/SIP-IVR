#ifndef MEDIASERVER_H
#define MEDIASERVER_H

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 9999

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
    RequestHandler _requestHandler;
};

#endif // MEDIASERVER_H
