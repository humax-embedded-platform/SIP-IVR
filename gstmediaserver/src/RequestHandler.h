#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H


#include <string>

#define REQUEST_SUCCESS "success"
#define REQUEST_TYPE "type"
#define REQUEST_DATA "data"
#define REQUEST_SESSION "session"
#define REQUEST_ERROR "error"

#define REMOTE_HOST "remote_host"
#define REMOTE_PORT "remote_port"
#define REMOTE_MEDIA_DESC "remote_media_desc"
#define PLAYBACK_SOURCE "playback_source"

#define REQUEST_TYPE_INIT_SESSION "init_session"
#define REQUEST_TYPE_START_SESSION "start_session"
#define REQUEST_TYPE_STOP_SESSION "stop_session"
#define REQUEST_TYPE_CLOSE_SESSION "close_session"
#define REQUEST_TYPE_UPDATE_SESSION "update_session"
#define REQUEST_TYPE_GET_DTMF_EVENT "get_dtmf_event"

namespace gstmediaserver {

typedef struct Request {
    std::string type;
    std::string sessionID;
    std::string data;
} Request;

typedef struct Response {
    bool success;
    std::string type;
    std::string sessionID;
    std::string data;
    std::string error;
} Response;

class RequestHandler
{
public:
    RequestHandler();

    void handleRequest(int conectionFd, const char* buffer, int len);

private:
    bool parseRequest(const char* buffer, int len, Request&);
    void sendResponse(int conectionFd, const Response& res);
};

} // namespace gstmediaserver

#endif // REQUESTHANDLER_H
