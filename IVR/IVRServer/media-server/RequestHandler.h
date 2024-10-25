#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#define REQUEST_TYPE "type"
#define REQUEST_DATA "data"

#define REQUEST_TYPE_INIT_SESSION "init_session"


class RequestHandler
{
public:
    RequestHandler();

    void handleRequest(int conectionFd, const char* buffer, int len);
};

#endif // REQUESTHANDLER_H
