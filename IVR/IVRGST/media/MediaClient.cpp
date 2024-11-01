#include "MediaClient.h"
#include "Log.hpp"
#include "MediaSession.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <chrono>

#define BUFFER_SIZE 1024
#define REQUEST_TIMEOUT 30 // 30 seconds


std::atomic<bool> running(true);

typedef struct Connection {
    int sock;
#if 0
    std::mutex mtx;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lck(mtx);
    std::shared_ptr<std::thread> listener;
    char buffer[BUFFER_SIZE] = {0};
    bool success = false;

    void onReadData() {
        memset(buffer, 0, BUFFER_SIZE); // Clear the buffer

        // Read response from the server

        cv.notify_one();
    }

    void read() {
        listener = std::make_shared<std::thread>([this]() {
            onDataReady();
        });
        if (cv.wait_for(lck, std::chrono::seconds(REQUEST_TIMEOUT)) == std::cv_status::timeout) {
            LOG_E << "Request timed out" << ENDL;
        }
    }
#endif
} Connection;

MediaClient::MediaClient() {

}

MediaClient::~MediaClient()
{
    running = false;
    _listenerThread->join();
}

void MediaClient::start()
{

}

bool MediaClient::initSession(std::shared_ptr<MediaSession> session)
{
    // Prepare the request
    json data;
    data[REMOTE_HOST] = session->remoteHost();
    data[REMOTE_PORT] = session->remotePort();
    data[REMOTE_MEDIA_DESC] = session->getMediaDescription();
    data[PLAYBACK_SOURCE] = session->getPbSourceFile();

    Request req;
    Response res;
    req.type = REQUEST_TYPE_INIT_SESSION;
    req.data = data.dump();
    if (!sendRequest(req, res)) {
        LOG_E << "Failed to send request" << ENDL;
    } else {
        if (res.success && res.sessionID.length() > 0) {
            session->setSessionID(res.sessionID);
            return true;
        } else {
            LOG_E << "Failed to initialize session: " << res.error << ENDL;
        }
    }
    LOG_E << "Failed to initialize session" << ENDL;
    return false;
}

bool MediaClient::startSession(std::shared_ptr<MediaSession> session)
{
    json data;
    data[REMOTE_HOST] = session->remoteHost();
    data[REMOTE_PORT] = session->remotePort();
    data[REMOTE_MEDIA_DESC] = session->getMediaDescription();
    data[PLAYBACK_SOURCE] = session->getPbSourceFile();

    Request req;
    Response res;
    req.type = REQUEST_TYPE_START_SESSION;
    req.sessionID = session->getSessionID();
    req.data = data.dump();
    if (!sendRequest(req, res)) {
        LOG_E << "Failed to send request" << ENDL;
    } else {
        if (res.success) {
            return true;
        } else {
            LOG_E << "Failed to start session: " << res.error << ENDL;
        }
    }
    return false;
}

bool MediaClient::stopSession(std::shared_ptr<MediaSession> session)
{
    Request req;
    Response res;
    req.type = REQUEST_TYPE_STOP_SESSION;
    req.sessionID = session->getSessionID();
    if (!sendRequest(req, res)) {
        LOG_E << "Failed to send request" << ENDL;
    } else {
        if (res.success) {
            return true;
        } else {
            LOG_E << "Failed to stop session: " << res.error << ENDL;
        }
    }
    return false;
}

bool MediaClient::closeSession(std::shared_ptr<MediaSession> session)
{
    Request req;
    Response res;
    req.type = REQUEST_TYPE_CLOSE_SESSION;
    req.sessionID = session->getSessionID();
    if (!sendRequest(req, res)) {
        LOG_E << "Failed to send request" << ENDL;
    } else {
        if (res.success) {
            return true;
        } else {
            LOG_E << "Failed to close session: " << res.error << ENDL;
        }
    }
    return false;
}

bool MediaClient::updateSession(std::shared_ptr<MediaSession> session)
{
    json data;
    data[REMOTE_HOST] = session->remoteHost();
    data[REMOTE_PORT] = session->remotePort();
    data[REMOTE_MEDIA_DESC] = session->getMediaDescription();
    data[PLAYBACK_SOURCE] = session->getPbSourceFile();

    Request req;
    Response res;
    req.type = REQUEST_TYPE_UPDATE_SESSION;
    req.sessionID = session->getSessionID();
    req.data = data.dump();
    if (!sendRequest(req, res)) {
        LOG_E << "Failed to send request" << ENDL;
    } else {
        if (res.success) {
            return true;
        } else {
            LOG_E << "Failed to update session: " << res.error << ENDL;
        }
    }
    return false;
}

std::string MediaClient::readDTMF(std::shared_ptr<MediaSession> session)
{
    Request req;
    Response res;
    req.type = REQUEST_TYPE_GET_DTMF_EVENT;
    req.sessionID = session->getSessionID();
    if (!sendRequest(req, res)) {
        LOG_E << "Failed to send request" << ENDL;
    } else {
        if (res.success) {
            return res.data;
        } else {
            LOG_E << "Failed to update session: " << res.error << ENDL;
        }
    }
    return "";
}

bool MediaClient::sendRequest(Request &req, Response &res)
{
    // LOG_I << "Sending request: " << req.payload() << ENDL;
    bool succes = false;
    std::shared_ptr<Connection> conn = std::make_shared<Connection>();

    // Create socket
    if ((conn->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG_E << "Socket creation error" << ENDL;
        return false;
    }

    _serv_addr.sin_family = AF_INET; // IPv4
    _serv_addr.sin_port = htons(DEFAULT_PORT); // Port

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, DEFAULT_HOST, &_serv_addr.sin_addr) <= 0) {
        LOG_E << "Invalid address/Address not supported" << ENDL;
        return false;
    }

    // Connect to the server
    if (connect(conn->sock, (struct sockaddr *)&_serv_addr, sizeof(_serv_addr)) < 0) {
        LOG_E << "Connection Failed" << ENDL;
        return false;
    }

    // Send the request
    std::string message = req.payload();
    send(conn->sock, message.c_str(), message.size(), 0);


    char buffer[BUFFER_SIZE] = {0};
    memset(buffer, 0, BUFFER_SIZE);

    // Read response from the server
    int bytesReceived = read(conn->sock, buffer, BUFFER_SIZE);
    if (bytesReceived > 0) {
        LOG_I << "Response from server: " << buffer << ENDL;
        res.parse(buffer);
        succes = true;
    } else if (bytesReceived == 0) {
        LOG_E << "Server closed the connection" << ENDL;
    } else {
        LOG_E << "Read failed" << ENDL;
    }

    close(conn->sock);
    return succes;
}
