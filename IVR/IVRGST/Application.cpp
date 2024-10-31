#include "Application.hpp"
#include "Log.hpp"
#include "SipMessageTypes.h"
#include "IDGen.hpp"
#include "AppDefines.h"
#include "session/CallSession.hpp"
#include "session/SessionManager.h"
#include "media/MediaSession.h"
#include "media/MediaManager.h"


Application::Application(std::string server_ip, int server_port, std::string app_ip, int app_port)
    : _server_ip(server_ip),
    _server_port(server_port),
    _app_ip(app_ip),
    _app_port(app_port),
    _server(server_ip, server_port, std::bind(&Application::onNewMessage, this, std::placeholders::_1, std::placeholders::_2)) {
    LOG_W << "Application created: " << server_ip << ":" << server_port << ", " << app_ip << ":" << app_port << ENDL;
    _server.startReceive();
    _handlers.emplace(SipMessageTypes::CANCEL, std::bind(&Application::OnCancel, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::INVITE, std::bind(&Application::OnInvite, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::TRYING, std::bind(&Application::OnTrying, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::RINGING, std::bind(&Application::OnRinging, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::BUSY, std::bind(&Application::OnBusy, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::UNAVAIALBLE, std::bind(&Application::OnUnavailable, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::OK, std::bind(&Application::OnOk, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::ACK, std::bind(&Application::OnAck, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::BYE, std::bind(&Application::OnBye, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::REQUEST_TERMINATED, std::bind(&Application::onReqTerminated, this, std::placeholders::_1));

    _registerThread = std::thread([this]() {
        while (true) {
            // send register message every 60 seconds
            std::shared_ptr<SipMessage> message = std::make_shared<SipMessage>();
            message->setHeader(std::string("REGISTER sip:") + _server.getIp() + " SIP/2.0");
            message->setTo(std::string(IVR_ACCOUNT_NAME) + "<" + std::string("sip:") + IVR_ACCOUNT_NAME + "@" + _server.getIp() + ">");
            message->setFrom(std::string(IVR_ACCOUNT_NAME) + "<" + std::string("sip:") + IVR_ACCOUNT_NAME + "@" + _server.getIp() + ">;tag=" + getAppTag());
            message->setCallID(IDGen::GenerateID(9));
            message->setCSeq("1 REGISTER");
            message->setVia(std::string("SIP/2.0/UDP ") + _server.getIp() + ":" + std::to_string(_server.getPort()));
            message->setContact(std::string("<sip:") + IVR_ACCOUNT_NAME + "@" + _server.getIp() + ":" + std::to_string(_server.getPort()) + ">");
            sendToServer(message);
            std::this_thread::sleep_for(std::chrono::seconds(60));
        }
    });
    _dtmfHandler = std::make_shared<DTMFHandler>(this);
}

std::string Application::serverHost() const
{
    return _server_ip;
}

int Application::serverPort() const
{
    return _server_port;
}

std::string Application::getAppIP() const
{
    return _app_ip;
}

int Application::getAppPort() const
{
    return _app_port;
}

void Application::onNewMessage(std::string data, sockaddr_in src)
{
    auto message = _messagesFactory.createMessage(std::move(data));
    if (message.has_value()) {
        auto request = message.value();
        if (_handlers.find(request->getType()) != _handlers.end()) {
            _handlers[request->getType()](std::move(request));
        }
    }
}

void Application::sendToServer(std::shared_ptr<SipMessage> message)
{
    _server.send(message->toPayload());
}

void Application::OnCancel(std::shared_ptr<SipMessage> data)
{

}

void Application::onReqTerminated(std::shared_ptr<SipMessage> data)
{

}

void Application::OnInvite(std::shared_ptr<SipMessage> data)
{
    std::shared_ptr<SipSdpMessage> sdp = std::dynamic_pointer_cast<SipSdpMessage>(data);
    std::string callID = sdp->getCallID();
    LOG_I << "Call ID: " << callID << ENDL;
    std::shared_ptr<CallSession> callSession = SessionManager::getInstance()->getSession(callID);
    if (callSession) {
        LOG_D << "Session already exists" << ENDL;
        return;
    } else {
        callSession = SessionManager::getInstance()->createSession(callID);
        std::string fromNUmber = sdp->getFromNumber();
        std::string toNumber = sdp->getToNumber();
        std::string fromTag = sdp->getFromTag();
        std::string toTag = sdp->getToTag();
        std::shared_ptr<SipClient> src = std::make_shared<SipClient>(fromNUmber, sdp->getSrc());
        callSession->setSrc(src, sdp->getRtpPort());
        callSession->setFromTag(fromTag);
        LOG_I << "From: " << fromNUmber << ", To: " << toNumber << ", FromTag: " << fromTag << ", ToTag: " << toTag << ENDL;

        std::shared_ptr<MediaSession> mediaSession = MediaManager::getInstance()->createSession(sdp->getRtpHost(), sdp->getRtpPort(), sdp->mediaDescription());
        if (mediaSession) {
            callSession->setMediaSession(mediaSession);
            callSession->setState(CallSession::State::Connected);
            mediaSession->setMediaSessionCallback(_dtmfHandler);
        } else {
            LOG_E << "Failed to create media session -> remove session" << ENDL;
            SessionManager::getInstance()->removeSession(callSession);
            return;
        }
    }

    // response to INVITE
    std::shared_ptr<SipSdpMessage> response = sdp;
    response->setHeader(std::string("SIP/2.0 200 OK"));
    response->setTo(std::string(IVR_ACCOUNT_NAME) + "<" + std::string("sip:") + IVR_ACCOUNT_NAME + "@" + _server.getIp() + ">;tag=" + getAppTag());
    response->setContact(std::string("<sip:") + IVR_ACCOUNT_NAME + "@" + _server.getIp() + ":" + std::to_string(_server.getPort()) + ">");

    response->setRtpHost(_app_ip);
    response->setRtpPort(_app_port);
    sendToServer(response);
}

void Application::OnTrying(std::shared_ptr<SipMessage> data)
{

}

void Application::OnRinging(std::shared_ptr<SipMessage> data)
{

}

void Application::OnBusy(std::shared_ptr<SipMessage> data)
{

}

void Application::OnUnavailable(std::shared_ptr<SipMessage> data)
{

}

void Application::OnBye(std::shared_ptr<SipMessage> data)
{
    LOG_I << ENDL;
    std::shared_ptr<CallSession> callSession = SessionManager::getInstance()->getSession(data->getCallID());
    if (callSession) {
        // Send 200 OK response
        std::shared_ptr<SipMessage> response = data;
        response->setHeader(std::string("SIP/2.0 200 OK"));
        sendToServer(response);

        // Remove all related sessions
        callSession->setState(CallSession::State::Bye);
        std::shared_ptr<MediaSession> mediaSession = callSession->getMediaSession();
        SessionManager::getInstance()->removeSession(callSession);
        if (mediaSession) {
            MediaManager::getInstance()->stopMediaSession(mediaSession);
            MediaManager::getInstance()->removeSession(mediaSession);
        } else {
            LOG_E << "MediaSession not found" << ENDL;
        }
    }
}


void Application::OnOk(std::shared_ptr<SipMessage> data)
{
    LOG_I << ENDL;
    std::string cSqe = data->getCSeq();
    if (cSqe.find("REGISTER") != std::string::npos) {
        LOG_D << "Register successed" << ENDL;
    } else {
        LOG_D << "Unknown OK message" << ENDL;
    }
}

void Application::OnAck(std::shared_ptr<SipMessage> data)
{
    LOG_I << "OnAck: " << ENDL;
    std::shared_ptr<CallSession> callSession = SessionManager::getInstance()->getSession(data->getCallID());
    if (callSession) {
        callSession->setState(CallSession::State::Connected);
        std::shared_ptr<MediaSession> mediaSession = callSession->getMediaSession();
        if (mediaSession) {
            mediaSession->setPbSourceFile("~/WorkSpace/SipServer/Blob/welcome.wav");
            MediaManager::getInstance()->startMediaSession(mediaSession);
        } else {
            LOG_E << "MediaSession not found" << ENDL;
        }
    }
}

std::string Application::getAppTag()
{
    static std::string tag = IDGen::GenerateID(9);
    return tag;
}
