#ifndef MEDIASERVER_H
#define MEDIASERVER_H


#include "Defines.h"
#include <thread>
#include <memory>

namespace gstmediaserver {

class RequestHandler;

class MediaServer
{
public:
    MediaServer(int port = DEFAULT_PORT);
    ~MediaServer();

    void startServer();
    void handleConnection(int clientFd);

private:
    void onStartServer();
    // void parse
private:
    bool _running;
    int _serverFd;
    int _serverPort;
    std::shared_ptr<std::thread> _serverThread;
    std::shared_ptr<RequestHandler> _requestHandler;
};

} // namespace gstmediaserver

#endif // MEDIASERVER_H
