#include "Application.hpp"
#include "Log.hpp"
#include "SipMessageTypes.h"
#include "IDGen.hpp"
#include "AppDefines.h"
#include "session/CallSession.hpp"
#include "session/SessionManager.h"
#include "media/MediaSession.h"
#include "media/MediaManager.h"
#include <cstdlib>
#include <filesystem>
#include <random>
#include <sstream>

std::string generateBranch() {
    // Start with the "magic cookie"
    std::string branch = "z9hG4bK-";

    // Use a random device and generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 15);

    // Generate a 32-character hexadecimal string
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << dist(gen);
    }

    // Append the generated string to the branch cookie
    branch += ss.str();
    return branch;
}

namespace gstivr {

Application::Application(std::string server_ip, int server_port, std::string app_ip, int app_rtp_port)
    : _server_ip(server_ip),
    _server_port(server_port),
    _app_ip(app_ip),
    _app_rtp_port(app_rtp_port),
    _server(server_ip, server_port, std::bind(&Application::onNewMessage, this, std::placeholders::_1, std::placeholders::_2)) {
    Logger::getLogger()->info("Application created: {}:{}, {}:{}", server_ip, server_port, app_ip, app_rtp_port );
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
    _handlers.emplace(SipMessageTypes::REFER_ACCEPTED, std::bind(&Application::OnReferAccepted, this, std::placeholders::_1));

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

    std::filesystem::path media_dir(getenv("MEDIA_DIR"));
    Logger::getLogger()->info("Media dir: {}", media_dir.string());
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

int Application::getAppRtpPort() const
{
    return _app_rtp_port;
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
    Logger::getLogger()->info("Call ID: {}", callID);
    std::shared_ptr<CallSession> callSession = SessionManager::getInstance()->getSession(callID);
    if (callSession) {
        Logger::getLogger()->info("Session already exists");
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

        std::shared_ptr<MediaSession> mediaSession = MediaManager::getInstance()->createSession(sdp->getRtpHost(), sdp->getRtpPort(), sdp->mediaDescription());
        if (mediaSession) {
            callSession->setMediaSession(mediaSession);
            callSession->setState(CallSession::State::Connected);
            mediaSession->setMediaSessionCallback(_dtmfHandler);
        } else {
            Logger::getLogger()->error("Failed to create media session -> remove session");
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
    response->setRtpPort(_app_rtp_port);
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
    Logger::getLogger()->info("OnUnavailable: {}", data->getCallID());
    std::shared_ptr<CallSession> callSession = SessionManager::getInstance()->getSession(data->getCallID());
    if (callSession) {
        std::shared_ptr<MediaSession> mediaSession = callSession->getMediaSession();
        if (mediaSession) {
            std::filesystem::path media_dir(getenv("MEDIA_DIR"));
            std::filesystem::path media_file = media_dir / "agent_busy.wav";
            mediaSession->setPbSourceFile(media_file.string());
            MediaManager::getInstance()->updateMediaSession(mediaSession);

            //delay for 7 seconds
            std::this_thread::sleep_for(std::chrono::seconds(7));

            // Send BYE to server
            std::shared_ptr<SipMessage> bye = std::make_shared<SipMessage>();
            bye->setHeader(std::string("BYE sip:") + callSession->getSrc()->getNumber() + "@" + _server.getIp() + ":" + std::to_string(_server.getPort()) + " SIP/2.0");
            bye->setTo(std::string("<sip:") + callSession->getSrc()->getNumber() + "@" + _server.getIp() + ">;tag=" + callSession->getFromTag());
            bye->setFrom(std::string("<sip:") + IVR_ACCOUNT_NAME + "@" + _server.getIp() + ">;tag=" + getAppTag() );
            bye->setVia(std::string("SIP/2.0/UDP ") + _app_ip + ":" + std::to_string(_app_rtp_port) + ";branch=" + generateBranch());
            bye->setCallID(callSession->getCallID());
            bye->setCSeq("2 BYE");
            bye->setContact(std::string("<sip:") + IVR_ACCOUNT_NAME + "@" + _server.getIp() + ":" + std::to_string(_server.getPort()) + ">");
            bye->setContentLength("0");
            sendToServer(bye);

            MediaManager::getInstance()->stopMediaSession(mediaSession);
            MediaManager::getInstance()->removeSession(mediaSession);
        } else {
            Logger::getLogger()->error("MediaSession not found");
        }
    }
}

void Application::OnBye(std::shared_ptr<SipMessage> data)
{
    Logger::getLogger()->info("OnBye: {}", data->getCallID());
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
            Logger::getLogger()->error("MediaSession not found");
        }
    }
}


void Application::OnOk(std::shared_ptr<SipMessage> data)
{
    Logger::getLogger()->info("OnOk: {}", data->getCallID());
    std::string cSqe = data->getCSeq();
    if (cSqe.find(SipMessageTypes::REGISTER) != std::string::npos) {
        Logger::getLogger()->info("Register successed");
    } else if (cSqe.find(SipMessageTypes::BYE) != std::string::npos) {
        OnBye(data);
    } else {
        Logger::getLogger()->warn("Unknown OK message");
    }
}

void Application::OnAck(std::shared_ptr<SipMessage> data)
{
    Logger::getLogger()->info("OnAck: {}", data->getCallID());
    std::shared_ptr<CallSession> callSession = SessionManager::getInstance()->getSession(data->getCallID());
    if (callSession) {
        callSession->setState(CallSession::State::Connected);
        std::shared_ptr<MediaSession> mediaSession = callSession->getMediaSession();
        if (mediaSession) {
            std::filesystem::path media_dir(getenv("MEDIA_DIR"));
            std::filesystem::path media_file = media_dir / "welcome.wav";
            mediaSession->setPbSourceFile(media_file.string());
            MediaManager::getInstance()->startMediaSession(mediaSession);
        } else {
            Logger::getLogger()->error("MediaSession not found");
        }
    }
}

void Application::OnReferAccepted(std::shared_ptr<SipMessage> data)
{
    std::string callId = data->getCallID();
    Logger::getLogger()->info("OnReferAccepted: {}", callId);
    std::shared_ptr<CallSession> callSession = SessionManager::getInstance()->getSession(callId);
    if (callSession) {
        callSession->setState(CallSession::State::Connected);
        std::shared_ptr<MediaSession> mediaSession = callSession->getMediaSession();
        if (mediaSession) {
            std::filesystem::path media_dir(getenv("MEDIA_DIR"));
            std::filesystem::path media_file = media_dir / "redirecting.wav";
            mediaSession->setPbSourceFile(media_file.string());
            MediaManager::getInstance()->updateMediaSession(mediaSession);

            std::this_thread::sleep_for(std::chrono::seconds(5));

            media_file = media_dir / "phone_holding_music.wav";
            mediaSession->setPbSourceFile(media_file.string());
            MediaManager::getInstance()->updateMediaSession(mediaSession);
        } else {
            Logger::getLogger()->error("MediaSession not found");
        }
    } else {
        Logger::getLogger()->error("CallSession not found");
    }
}

std::string Application::getAppTag()
{
    static std::string tag = IDGen::GenerateID(9);
    return tag;
}
} // namespace gstivr
