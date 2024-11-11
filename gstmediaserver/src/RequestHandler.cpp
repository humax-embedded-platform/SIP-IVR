#include "RequestHandler.h"
#include "util/Log.hpp"
#include "SessionManager.h"
#include "json.hpp"
#include "MediaSession.h"
#include <sys/socket.h>

namespace gstmediaserver {

using json = nlohmann::json;

RequestHandler::RequestHandler() {

}

void RequestHandler::handleRequest(int conectionFd, const char *buffer, int len)
{
    Request req;
    if (!parseRequest(buffer, len, req))
    {
        Logger::getLogger()->error("Invalid request");
        Response res;
        res.success = false;
        res.type = req.type;
        res.sessionID = req.sessionID;
        res.error = "Invalid request";
        sendResponse(conectionFd, res);
        return;
    }

    if (req.type == REQUEST_TYPE_INIT_SESSION) {
        Logger::getLogger()->info("Handling init_session request ...");
        json data = json::parse(req.data);
        auto _sessionManager = SessionManager::getInstance();
        std::shared_ptr<MediaSession> session = _sessionManager->createSession(data[REMOTE_HOST], data[REMOTE_PORT]);
        Response res;
        if (session == nullptr) {
            res.success = false;
            res.type = req.type;
            res.sessionID = "";
            res.error = "Failed to create session";
        } else {
            if (data.contains(REMOTE_MEDIA_DESC)) {
                std::string mediaDesc = data[REMOTE_MEDIA_DESC];
                session->setMediaDescription(mediaDesc);
            }

            if (data.contains(PLAYBACK_SOURCE)) {
                std::string pbSource = data[PLAYBACK_SOURCE];
                session->setPBSourceFile(pbSource);
            }
            session->open();

            res.success = true;
            res.type = req.type;
            res.sessionID = session->sessionID();
            res.data = session->sessionID();
        }
        Logger::getLogger()->info("init_session response: {}", res.sessionID);
        sendResponse(conectionFd, res);
    } else if (req.type == REQUEST_TYPE_START_SESSION) {
        Logger::getLogger()->info("Handling start_session request ... {}", req.sessionID);
        auto _sessionManager = SessionManager::getInstance();
        std::shared_ptr<MediaSession> session = _sessionManager->getSession(req.sessionID);
        Response res;
        if (session == nullptr) {
            res.success = false;
            res.type = req.type;
            res.sessionID = req.sessionID;
            res.error = "Session not found";
            sendResponse(conectionFd, res);
            return;
        }
        // start session
        json data = json::parse(req.data);
        if (data.contains(PLAYBACK_SOURCE)) {
            std::string pbSource = data[PLAYBACK_SOURCE];
            session->setPBSourceFile(pbSource);
        }
        session->start();
        res.success = true;
        res.type = req.type;
        res.sessionID = req.sessionID;
        res.data = "Session started";
        sendResponse(conectionFd, res);
    } else if (req.type == REQUEST_TYPE_STOP_SESSION) {
        Logger::getLogger()->info("Handling stop_session request ...{}", req.sessionID);
        auto _sessionManager = SessionManager::getInstance();
        std::shared_ptr<MediaSession> session = _sessionManager->getSession(req.sessionID);
        Response res;
        if (session == nullptr) {
            res.success = false;
            res.type = req.type;
            res.sessionID = req.sessionID;
            res.error = "Session not found";
            sendResponse(conectionFd, res);
            return;
        }

        // stop session
        session->stop();
        res.success = true;
        res.type = req.type;
        res.sessionID = req.sessionID;
        res.data = "Session stopped";
        sendResponse(conectionFd, res);
    } else if (req.type == REQUEST_TYPE_UPDATE_SESSION) {
        Logger::getLogger()->info("Handling update_session request ...{}", req.sessionID);
        auto _sessionManager = SessionManager::getInstance();
        std::shared_ptr<MediaSession> session = _sessionManager->getSession(req.sessionID);
        Response res;
        if (session == nullptr) {
            res.success = false;
            res.type = req.type;
            res.sessionID = req.sessionID;
            res.error = "Session not found";
            sendResponse(conectionFd, res);
            return;
        }

        json data = json::parse(req.data);
        if (data.contains(REMOTE_MEDIA_DESC)) {
            session->setMediaDescription(data[REMOTE_MEDIA_DESC]);
        }

        if (data.contains(PLAYBACK_SOURCE)) {
            session->setPBSourceFile(data[PLAYBACK_SOURCE]);
        }

        res.success = true;
        res.type = req.type;
        res.sessionID = req.sessionID;
        res.data = "Session updated";
        sendResponse(conectionFd, res);
    } else if (req.type == REQUEST_TYPE_CLOSE_SESSION) {
        Logger::getLogger()->info("Handling close_session request ...{}", req.sessionID);
        auto _sessionManager = SessionManager::getInstance();
        std::shared_ptr<MediaSession> session = _sessionManager->getSession(req.sessionID);
        Response res;
        if (session == nullptr) {
            res.success = false;
            res.type = req.type;
            res.sessionID = req.sessionID;
            res.error = "Session not found";
            sendResponse(conectionFd, res);
            return;
        }

        _sessionManager->removeSession(req.sessionID);
        res.success = true;
        res.type = req.type;
        res.sessionID = req.sessionID;
        res.data = "Session stopped";
        sendResponse(conectionFd, res);
    } else if (req.type == REQUEST_TYPE_GET_DTMF_EVENT) {
        // Logger::getLogger()->info("Handling get_dtmf_event request ...{}", req.sessionID);
        auto _sessionManager = SessionManager::getInstance();
        std::shared_ptr<MediaSession> session = _sessionManager->getSession(req.sessionID);
        Response res;
        if (session == nullptr) {
            res.success = false;
            res.type = req.type;
            res.sessionID = req.sessionID;
            res.error = "Session not found";
            sendResponse(conectionFd, res);
            return;
        }

        std::string dtmfEvent = session->getDTMFEvent();
        res.success = true;
        res.type = req.type;
        res.sessionID = req.sessionID;
        res.data = dtmfEvent;
        sendResponse(conectionFd, res);
        session->clearDTMFEvent();
    } else {
        Logger::getLogger()->error("Invalid request type");
        Response res;
        res.success = false;
        res.type = req.type;
        res.sessionID = req.sessionID;
        res.error = "Invalid request type";
        sendResponse(conectionFd, res);
    }
}

bool RequestHandler::parseRequest(const char *buffer, int len, Request &req)
{
    bool success = false;
    json reqJson = json::parse(buffer);
    // Logger::getLogger()->info("Request: {}", reqJson.dump());
    if (reqJson.contains(REQUEST_TYPE)) {
        success = true;
        req.type = reqJson[REQUEST_TYPE];
    }

    if (reqJson.contains(REQUEST_DATA)) {
        success = true;
        req.data = reqJson[REQUEST_DATA].dump();
    }

    if (success && reqJson.contains(REQUEST_SESSION)) {
        req.sessionID = reqJson[REQUEST_SESSION];
    }
    return success;
}

void RequestHandler::sendResponse(int conectionFd, const Response &res)
{
    json resJson;
    resJson[REQUEST_SUCCESS] = res.success;
    resJson[REQUEST_TYPE] = res.type;
    resJson[REQUEST_SESSION] = res.sessionID;
    resJson[REQUEST_DATA] = res.data;
    resJson[REQUEST_ERROR] = res.error;
    std::string response = resJson.dump();
    // Logger::getLogger()->info("Sending response ...{}", response);
    send(conectionFd, response.c_str(), response.size(), 0);
}

} // namespace gstmediaserver
