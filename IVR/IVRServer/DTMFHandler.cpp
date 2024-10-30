#include "DTMFHandler.h"
#include "session/SessionManager.h"
#include "session/CallSession.hpp"
#include "Log.hpp"

DTMFHandler::DTMFHandler() {}

void DTMFHandler::onDTMFEvent(std::shared_ptr<MediaSession> session, std::string event)
{
    LOG_D << "DTMF event: " << event << ENDL;
    std::shared_ptr<CallSession> callSession = SessionManager::getInstance()->findSession(session);
}
