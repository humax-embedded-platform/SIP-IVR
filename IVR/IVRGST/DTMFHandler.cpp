#include "DTMFHandler.h"
#include "session/SessionManager.h"
#include "session/CallSession.hpp"
#include "MediaManager.h"
#include "Log.hpp"
#include "SipSdpMessage.hpp"
#include "Application.hpp"
#include "AppDefines.h"
#include <filesystem>
#include <chrono>

DTMFHandler::DTMFHandler(Application* app) : _app(app) {}

void DTMFHandler::onDTMFEvent(std::shared_ptr<MediaSession> session, std::string event)
{
    std::shared_ptr<CallSession> callSession = SessionManager::getInstance()->findSession(session);
    int number = std::stoi(event);
    Logger::getLogger()->info("DTMF number: {}", number);
    switch (number) {
    case KEY_0:
        replayGuide(callSession);
        break;
    case KEY_1:
        makeRefer("agent1", callSession);
        break;
    case KEY_2:
        makeRefer("agent2", callSession);
        break;
    case KEY_3:
        makeRefer("agent3", callSession);
        break;
    case KEY_4:
        makeRefer("agent4", callSession);
        break;
    case KEY_5:
        makeRefer("agent5", callSession);
        break;
    default:
        invalidChoose(callSession);
        break;
    }
}

void DTMFHandler::makeRefer(std::string agentID, std::shared_ptr<CallSession> session)
{
    Logger::getLogger()->info("Making refer to agent: {}", agentID);

    // Make refer to agent
    std::shared_ptr<SipMessage> refer = std::make_shared<SipMessage>();
    refer->setHeader(std::string("REFER sip:") + session->getSrc()->getNumber() + "@" + _app->serverHost() + " SIP/2.0");
    refer->setVia(std::string("SIP/2.0/UDP ") + _app->serverHost() + ":" + std::to_string(_app->serverPort()));
    refer->setTo(session->getSrc()->getNumber() + "<" + std::string("sip:") + session->getSrc()->getNumber() + "@" + _app->serverHost() + ">;tag=" + session->getFromTag());
    refer->setFrom(std::string("<sip:") + IVR_ACCOUNT_NAME + "@" + _app->serverHost() + ">;tag=" + _app->getAppTag());
    refer->setCallID(session->getCallID());
    refer->setCSeq("1 REFER");
    refer->setReferTo("<" + std::string("sip:") + agentID + "@" + _app->serverHost() + ">");
    refer->setReferBy("<" + std::string("sip:") + IVR_ACCOUNT_NAME + "@" + _app->serverHost() + ">");
    refer->setContact(std::string("<sip:") + IVR_ACCOUNT_NAME + "@" + _app->getAppIP() + ":" + std::to_string(_app->getAppPort()) + ">");
    _app->sendToServer(refer);
}

void DTMFHandler::replayGuide(std::shared_ptr<CallSession> session)
{
    std::shared_ptr<MediaSession> mediaSession = session->getMediaSession();
    std::filesystem::path media_dir(getenv("MEDIA_DIR"));
    std::filesystem::path media_file = media_dir / "welcome.wav";
    mediaSession->setPbSourceFile(media_file.string());
    MediaManager::getInstance()->updateMediaSession(mediaSession);
}

void DTMFHandler::invalidChoose(std::shared_ptr<CallSession> session)
{
    std::shared_ptr<MediaSession> mediaSession = session->getMediaSession();
    std::filesystem::path media_dir(getenv("MEDIA_DIR"));
    std::filesystem::path media_file = media_dir / "selection_unavailable.wav";
    mediaSession->setPbSourceFile(media_file.string());
    MediaManager::getInstance()->updateMediaSession(mediaSession);

    // delay 5s
    std::this_thread::sleep_for(std::chrono::seconds(5));

    media_file = media_dir / "welcome.wav";
    mediaSession->setPbSourceFile(media_file.string());
    MediaManager::getInstance()->updateMediaSession(mediaSession);
}
