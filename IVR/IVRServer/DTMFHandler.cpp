#include "DTMFHandler.h"
#include "session/SessionManager.h"
#include "session/CallSession.hpp"
#include "MediaManager.h"
#include "Log.hpp"
#include "SipSdpMessage.hpp"

DTMFHandler::DTMFHandler(Application* app) : _app(app) {}

void DTMFHandler::onDTMFEvent(std::shared_ptr<MediaSession> session, std::string event)
{
    std::shared_ptr<CallSession> callSession = SessionManager::getInstance()->findSession(session);
    int number = std::stoi(event);
    LOG_D << "DTMF number: " << number << ENDL;
    switch (number) {
    case KEY_0:
        replayGuide();
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
        invalidChoose();
        break;
    }
}

void DTMFHandler::makeRefer(std::string agentID, std::shared_ptr<CallSession> session)
{
    LOG_D << "Making refer to agent: " << agentID << ENDL;
    std::shared_ptr<MediaSession> mediaSession = session->getMediaSession();
    mediaSession->setPbSourceFile("~/WorkSpace/SipServer/blob/redirecting.wav");
    MediaManager::getInstance()->updateMediaSession(mediaSession);


    // Make refer to agent
    std::shared_ptr<SipSdpMessage> refer = std::make_shared<SipSdpMessage>();
    refer->setMethod("REFER");

}

void DTMFHandler::replayGuide()
{

}

void DTMFHandler::invalidChoose()
{

}
