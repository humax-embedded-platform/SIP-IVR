#ifndef MEDIACLIENT_H
#define MEDIACLIENT_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <memory>
#include "json.hpp"

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 9999

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

using json = nlohmann::json;

typedef struct Request {
    std::string type;
    std::string sessionID;
    std::string data;

    std::string payload() {
        nlohmann::json req;
        req[REQUEST_TYPE] = type;
        req[REQUEST_SESSION] = sessionID;
        if (!data.empty()) {
            json data_json = json::parse(data);
            req[REQUEST_DATA] = data_json;
        }
        return req.dump();
    }
} Request;

typedef struct Response {
    bool success;
    std::string type;
    std::string sessionID;
    std::string data;
    std::string error;

    bool parse(const char* buffer) {
        json res = json::parse(buffer);
        if (!res.contains(REQUEST_SUCCESS) || !res.contains(REQUEST_TYPE)) {
            return false;
        }
        success = res[REQUEST_SUCCESS];
        type = res[REQUEST_TYPE];
        if (res.contains(REQUEST_SESSION)) {
            sessionID = res[REQUEST_SESSION];
        }
        if (res.contains(REQUEST_DATA)) {
            data = res[REQUEST_DATA];
        }
        if (res.contains(REQUEST_ERROR)) {
            error = res[REQUEST_ERROR];
        }
        return true;
    }
} Response;

class MediaSession;
class MediaClient
{
public:
    MediaClient();
    ~MediaClient();

    void start();
    bool initSession(std::shared_ptr<MediaSession> session);
    bool startSession(std::shared_ptr<MediaSession> session);
    bool stopSession(std::shared_ptr<MediaSession> session);
    bool closeSession(std::shared_ptr<MediaSession> session);
    bool updateSession(std::shared_ptr<MediaSession> session);
    std::string readDTMF(std::shared_ptr<MediaSession> session);

private:
    bool sendRequest(Request &req, Response &res);

private:
    struct sockaddr_in _serv_addr;
    std::shared_ptr<std::thread> _listenerThread;
};

#endif // MEDIACLIENT_H
