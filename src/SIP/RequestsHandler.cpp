#include "RequestsHandler.hpp"
#include <sstream>
#include "SipMessageTypes.h"
#include "SipSdpMessage.hpp"
#include "IDGen.hpp"
#include "Log.hpp"
#include "SipMessageFactory.hpp"

#define TEST_CALLER_IP "192.168.0.6"
#define TEST_CALLER_PORT "77777"

RequestsHandler::RequestsHandler(std::string serverIp, int serverPort,
    OnHandledEvent onHandledEvent) :
    _serverIp(std::move(serverIp)), _serverPort(serverPort),
    _onHandled(onHandledEvent)
{
    initHandlers();
}

void RequestsHandler::initHandlers()
{
    _handlers.emplace(SipMessageTypes::REGISTER, std::bind(&RequestsHandler::OnRegister, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::CANCEL, std::bind(&RequestsHandler::OnCancel, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::INVITE, std::bind(&RequestsHandler::OnInvite, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::TRYING, std::bind(&RequestsHandler::OnTrying, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::RINGING, std::bind(&RequestsHandler::OnRinging, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::BUSY, std::bind(&RequestsHandler::OnBusy, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::UNAVAIALBLE, std::bind(&RequestsHandler::OnUnavailable, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::OK, std::bind(&RequestsHandler::OnOk, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::ACK, std::bind(&RequestsHandler::OnAck, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::BYE, std::bind(&RequestsHandler::OnBye, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::REFER, std::bind(&RequestsHandler::OnRefer, this, std::placeholders::_1));
    _handlers.emplace(SipMessageTypes::REQUEST_TERMINATED, std::bind(&RequestsHandler::onReqTerminated, this, std::placeholders::_1));
}

void RequestsHandler::handle(std::shared_ptr<SipMessage> request)
{
    static int count = 0;
    // std::cout << "handle: " << request->toString() << std::endl;
    if (_handlers.find(request->getType()) != _handlers.end())
    {
        _handlers[request->getType()](std::move(request));
    }
    count++;
}

std::optional<std::shared_ptr<Session>> RequestsHandler::getSession(const std::string& callID)
{
    auto sessionIt = _sessions.find(callID);
    if (sessionIt != _sessions.end())
    {
        return sessionIt->second;
    }
    return {};
}

void RequestsHandler::OnRegister(std::shared_ptr<SipMessage> data)
{

    bool isUnregisterReq = data->getContact().find("expires=0") != -1;

    if (!isUnregisterReq)
    {
        auto newClient = std::make_shared<SipClient>(data->getFromNumber(), data->getSource());
        registerClient(std::move(newClient));
    }

    auto response = data;
    response->setHeader(SipMessageTypes::OK);
    response->setVia(data->getVia() + ";received=" + _serverIp);
    response->setTo(data->getTo() + ";tag=" + IDGen::GenerateID(9));
    response->setContact("Contact: <sip:" + data->getFromNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ";transport=UDP>");
    endHandle(response->getFromNumber(), response);

    if (isUnregisterReq)
    {
        auto newClient = std::make_shared<SipClient>(data->getFromNumber(), data->getSource());
        unregisterClient(std::move(newClient));
    }
}

void RequestsHandler::OnCancel(std::shared_ptr<SipMessage> data)
{
    setCallState(data->getCallID(), Session::State::Cancel);
    endHandle(data->getToNumber(), data);
}

void RequestsHandler::onReqTerminated(std::shared_ptr<SipMessage> data)
{
    endHandle(data->getFromNumber(), data);
}

void RequestsHandler::OnInvite(std::shared_ptr<SipMessage> data)
{
    LOG_I << "OnInvite: " << data->toString() << ENDL;
    // Check if the caller is registered
    auto caller = findClient(data->getFromNumber());
    if (!caller.has_value())
    {
        return;
    }

    // Check if the called is registered
    auto called = findClient(data->getToNumber());
    if (!called.has_value())
    {
        // Send "SIP/2.0 404 Not Found"
        data->setHeader(SipMessageTypes::NOT_FOUND);
        data->setContact("Contact: <sip:" + caller.value()->getNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ";transport=UDP>");
        endHandle(data->getFromNumber(), data);
        return;
    }

    auto message = dynamic_cast<SipSdpMessage*>(data.get());
    if (!message)
    {
        std::cerr << "Couldn't get SDP from " << data->getFromNumber() << "'s INVITE request." << std::endl;
        return;
    }

    auto newSession = std::make_shared<Session>(data->getCallID(), caller.value(), message->getRtpPort());
    _sessions.emplace(data->getCallID(), newSession);

    auto response = data;
    response->setContact("Contact: <sip:" + caller.value()->getNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ";transport=UDP>");
    endHandle(data->getToNumber(), response);
}

void RequestsHandler::OnTrying(std::shared_ptr<SipMessage> data)
{
    endHandle(data->getFromNumber(), data);
}

void RequestsHandler::OnRinging(std::shared_ptr<SipMessage> data)
{
    endHandle(data->getFromNumber(), data);
}

void RequestsHandler::OnBusy(std::shared_ptr<SipMessage> data)
{
    setCallState(data->getCallID(), Session::State::Busy);
    endHandle(data->getFromNumber(), data);
}

void RequestsHandler::OnUnavailable(std::shared_ptr<SipMessage> data)
{
    setCallState(data->getCallID(), Session::State::Unavailable);
    endHandle(data->getFromNumber(), data);
}

void RequestsHandler::OnBye(std::shared_ptr<SipMessage> data)
{
    setCallState(data->getCallID(), Session::State::Bye);
    endHandle(data->getToNumber(), data);
}

#define BUFFER_SIZE 4096

void RequestsHandler::OnRefer(std::shared_ptr<SipMessage> data)
{
    LOG_I << ENDL;
    auto refer = data;
    std::string orgCallId = refer->getCallID();
    auto session = getSession(orgCallId);
    if (!session.has_value())
    {
        return;
    }

    std::string replacedCallId = "";
    std::string replacedFromTag, replacedToTag;
    for (auto& [callId, _session] : _sessions)
    {
        if (_session->getSrc()->getNumber() == data->getFromNumber())
        {
            // LOG_D << "Found session for callID: " << callId << ENDL;
            // //remove "Call-ID: " in callId
            // replacedCallId = callId.substr(9);

            // replacedFromTag = _session->getFromTag();
            // replacedToTag = _session->getToTag();
            // break;
        }
    }

    std::string newCallId = "xxxxxxxxxxxxxxxxxxx";
    if (getSession("Call-ID: " + newCallId).has_value())
    {
        LOG_E << "Session already exists for callID: " << orgCallId << ENDL;
        return;
    }

    uint32_t port = session->get()->getSrcRtpPort();
    auto newSession = std::make_shared<Session>("Call-ID: " + newCallId, session.value()->getSrc(), port);
    _sessions.emplace("Call-ID: " + newCallId, newSession);

    SipMessageFactory factory;
    // Send a 202 Accepted response for the REFER.
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE,
             "SIP/2.0 202 Accepted\r\n"
             "Via: SIP/2.0/UDP " TEST_CALLER_IP ":"TEST_CALLER_PORT";branch=z9hG4bK1234\r\n"
             "From: <sip:phongivr@192.168.0.4>\r\n"
             "To: <sip:phong1@192.168.0.4>\r\n"
             "Call-ID: %s\r\n"
             "CSeq: 1 REFER\r\n"
             "Content-Length: 0\r\n\r\n",
             orgCallId.c_str());
    auto responseMsg = factory.createMessage(response, session.value()->getSrc()->getAddress());
    if (responseMsg.has_value())
    {
        endHandle("phongivr", responseMsg.value());
    }

    // Send invite to the refer target.
    char invite[BUFFER_SIZE];
    snprintf(invite, BUFFER_SIZE,
                "INVITE sip:phong2@192.168.0.4;transport=UDP SIP/2.0\r\n"
                "Via: SIP/2.0/UDP "TEST_CALLER_IP ":" TEST_CALLER_PORT ";branch=z9hG4bK-524287-1---c4ee7dee1e5d938d;rport\r\n"
                "Max-Forwards: 70\r\n"
                "Contact: <sip:phong1@"TEST_CALLER_IP":"TEST_CALLER_PORT";transport=UDP>\r\n"
                "To: <sip:phong2@192.168.0.4>\r\n"
                "From: <sip:phong1@192.168.0.4;transport=UDP>;tag=%s\r\n"
                "Call-ID: %s\r\n"
                "CSeq: 1 INVITE\r\n"
                "Allow: INVITE, ACK, CANCEL, BYE, NOTIFY, REFER, MESSAGE, OPTIONS, INFO, SUBSCRIBE\r\n"
                "Content-Type: application/sdp\r\n"
                "Supported: replaces, norefersub, extended-refer, timer, sec-agree, outbound, path, X-cisco-serviceuri\r\n"
                "User-Agent: Z 5.6.4 v2.10.20.4_1\r\n"
                "Allow-Events: presence, kpml, talk, as-feature-event\r\n"
                "Content-Length: 338\r\n"
                "\r\n"
                "v=0\r\n"
                "o=- 7223043 7223302 IN IP4 "TEST_CALLER_IP"\r\n"
                "s=eyeBeam\r\n"
                "c=IN IP4 "TEST_CALLER_IP"\r\n"
                "t=0 0\r\n"
                "m=audio %d RTP/AVP 100 6 0 8 3 18 5 101\r\n"
                "a=alt:1 1 : 03CBAE4D 00000018 "TEST_CALLER_IP" %d\r\n"
                "a=fmtp:101 0-15\r\n"
                "a=rtpmap:100 speex/16000\r\n"
                "a=rtpmap:101 telephone-event/8000\r\n"
                "a=sendrecv\r\n", session.value()->getFromTag().c_str(), newCallId.c_str(), port, port);
    auto inviteMsg = factory.createMessage(invite, session.value()->getSrc()->getAddress());
    endHandle("phong2", inviteMsg.value());
}

void RequestsHandler::OnOk(std::shared_ptr<SipMessage> data)
{
    LOG_I << ENDL;

    for (auto& [callId, _session] : _sessions)
    {
        if (_session->getSrc()->getNumber() == data->getFromNumber())
        {
            // LOG_D << "Found session for callID: " << callId << ENDL;
            // //remove "Call-ID: " in callId
            // replacedCallId = callId.substr(9);

            // replacedFromTag = _session->getFromTag();
            // replacedToTag = _session->getToTag();
            // break;
        }
    }

    bool replaced = false;
    std::string newCallId = "xxxxxxxxxxxxxxxxxxx";
    auto replacedSession = getSession("Call-ID: " + newCallId);
    if (replacedSession.has_value())
    {
        replaced = true;
    }

    auto session = getSession(data->getCallID());
    if (session.has_value())
    {
        if (session.value()->getState() == Session::State::Cancel)
        {
            endHandle(data->getFromNumber(), data);
            return;
        }

        if (data->getCSeq().find(SipMessageTypes::INVITE) != std::string::npos)
        {
            auto client = findClient(data->getToNumber());
            if (!client.has_value())
            {
                return;
            }

            auto sdpMessage = dynamic_cast<SipSdpMessage*>(data.get());
            if (!sdpMessage)
            {
                std::cerr << "Coudn't get SDP from: " << client.value()->getNumber() << "'s OK message.";
                endCall(data->getCallID(), data->getFromNumber(), data->getToNumber(), "SDP parse error.");
                return;
            }

            std::string _to = sdpMessage->getTo();
            if (_to.find("tag=") != std::string::npos) {
                std::string toTag = _to.substr(_to.find("tag=") + 4);
                session->get()->setToTag(toTag);
                // LOG_D << "To tag: " << toTag << ENDL;
            }

            std::string _from = sdpMessage->getFrom();
            if (_from.find("tag=") != std::string::npos) {
                std::string fromTag = _from.substr(_from.find("tag=") + 4);
                session->get()->setFromTag(fromTag);
                // LOG_D << "From tag: " << fromTag << ENDL;
            }

            session->get()->setDest(client.value(), sdpMessage->getRtpPort());
            session->get()->setState(Session::State::Connected);
            auto response = data;
            response->setContact("Contact: <sip:" + data->getToNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ";transport=UDP>");

            if (replaced) {
                data->setCallID("Call-ID: " + newCallId);
                data->setTo("To: <sip:" + replacedSession.value()->getDest()->getNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ";tag=" + replacedSession.value()->getToTag() + ">");
                endHandle(replacedSession.value()->getDest()->getNumber(), data);
            } else {
                endHandle(data->getFromNumber(), std::move(response));
            }

            if (data->getCallID().find("xxxxxxxxxxxxxxxxxxx") == std::string::npos) return;

        // Send invite to the refer target.
            std::string orginalCallID = "";
            std::string ivrTag = "";
            for (auto& [callId, _session] : _sessions)
            {
                if (_session->getDest()->getNumber() == "phongivr")
                {
                    // LOG_D << "Found session for callID: " << callId << ENDL;
                    // //remove "Call-ID: " in callId
                    orginalCallID = callId.substr(9);
                    ivrTag = _session->getToTag();
                    LOG_D << "Original callID: " << orginalCallID << ENDL;

                    // replacedFromTag = _session->getFromTag();
                    // replacedToTag = _session->getToTag();
                    // break;
                }
            }
            uint32_t port = session->get()->getDestRtpPort();
            char invite[BUFFER_SIZE];
            snprintf(invite, BUFFER_SIZE,
                        "INVITE sip:phong1@192.168.0.4;transport=UDP SIP/2.0\r\n"
                        "Via: SIP/2.0/UDP 192.168.0.4:5555;branch=z9hG4bK-524287-1---c4ee7dee1e5d938d;rport\r\n"
                        "Max-Forwards: 70\r\n"
                        "Contact: <sip:phongivr@192.168.0.4:5555;transport=UDP>\r\n"
                        "To: <sip:phong1@192.168.0.4>;tag=%s\r\n"
                        "From: <sip:phongivr@192.168.0.4;transport=UDP>;tag=%s\r\n"
                        "Call-ID: %s\r\n"
                        "CSeq: 2 INVITE\r\n"
                        "Allow: INVITE, ACK, CANCEL, BYE, NOTIFY, REFER, MESSAGE, OPTIONS, INFO, SUBSCRIBE\r\n"
                        "Content-Type: application/sdp\r\n"
                        "Supported: replaces, norefersub, extended-refer, timer, sec-agree, outbound, path, X-cisco-serviceuri\r\n"
                        "User-Agent: Z 5.6.4 v2.10.20.4_1\r\n"
                        "Allow-Events: presence, kpml, talk, as-feature-event\r\n"
                        "Content-Length: 338\r\n"
                        "\r\n"
                        "v=0\r\n"
                        "o=- 179839242 179839248 IN IP4 192.168.0.8\r\n"
                        "s=eyeBeam\r\n"
                        "c=IN IP4 192.168.0.8\r\n"
                        "t=0 0\r\n"
                        "m=audio %d RTP/AVP 100 6 0 8 3 18 5 101\r\n"
                        "a=alt:1 1 : 0FF5390C 000000AF 192.168.0.8 %d\r\n"
                        "a=fmtp:101 0-15\r\n"
                        "a=rtpmap:100 speex/16000\r\n"
                        "a=rtpmap:101 telephone-event/8000\r\n"
                        "a=sendrecv\r\n", session.value()->getFromTag().c_str(), ivrTag.c_str(), orginalCallID.c_str(), port, port);

            SipMessageFactory factory;
            auto inviteMsg = factory.createMessage(invite, session.value()->getSrc()->getAddress());
            endHandle("phong1", inviteMsg.value());
            return;
        } else {
            LOG_E << "Invalid CSeq: " << data->getCSeq() << ENDL;
        }

        if (session.value()->getState() == Session::State::Bye)
        {
            endHandle(data->getFromNumber(), data);
            endCall(data->getCallID(), data->getToNumber(), data->getFromNumber());
        }
    } else {
        LOG_E << "Session not found for callID: " << data->getCallID() << ENDL;
    }
}

void RequestsHandler::OnAck(std::shared_ptr<SipMessage> data)
{
    LOG_I << "OnAck: " << data->toString() << ENDL;
    auto session = getSession(data->getCallID());
    if (!session.has_value())
    {
        return;
    }

    bool replaced = false;
    std::string newCallId = "xxxxxxxxxxxxxxxxxxx";
    auto replacedSession = getSession("Call-ID: " + newCallId);
    if (replacedSession.has_value())
    {
        replaced = true;
    }

    if (replaced) {
        data->setCallID("Call-ID: " + newCallId);
        data->setTo("To: <sip:" + replacedSession.value()->getDest()->getNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ";tag=" + replacedSession.value()->getToTag() + ">");
        endHandle(replacedSession.value()->getDest()->getNumber(), data);
    } else {
        endHandle(data->getToNumber(), data);
    }

    auto sessionState = session.value()->getState();
    std::string endReason;
    if (sessionState == Session::State::Busy)
    {
        endReason = data->getToNumber() + " is busy.";
        endCall(data->getCallID(), data->getFromNumber(), data->getToNumber(), endReason);
        return;
    }

    if (sessionState == Session::State::Unavailable)
    {
        endReason = data->getToNumber() + " is unavailable.";
        endCall(data->getCallID(), data->getFromNumber(), data->getToNumber(), endReason);
        return;
    }

    if (sessionState == Session::State::Cancel)
    {
        endReason = data->getFromNumber() + " canceled the session.";
        endCall(data->getCallID(), data->getFromNumber(), data->getToNumber(), endReason);
        return;
    }
}

bool RequestsHandler::setCallState(const std::string& callID, Session::State state)
{
    auto session = getSession(callID);
    if (session)
    {
        session->get()->setState(state);
        return true;
    }

    return false;
}

void RequestsHandler::endCall(const std::string& callID, const std::string& srcNumber, const std::string& destNumber, const std::string& reason)
{
    LOG_W << "Ending call: " << callID << " between " << srcNumber << " and " << destNumber << ENDL;
    if (_sessions.erase(callID) > 0)
    {
        std::ostringstream message;
        message << "Session has been disconnected between " << srcNumber << " and " << destNumber;
        if (!reason.empty())
        {
            message << " because " << reason;
        }
        std::cout << message.str() << std::endl;
    }
}

bool RequestsHandler::registerClient(std::shared_ptr<SipClient> client)
{
    if (_clients.find(client->getNumber()) == _clients.end()) {
        std::cout << "New Client: " << client->getNumber() << std::endl;
        _clients.emplace(client->getNumber(), client);
        return true;
    } else {
        // replace ex
        _clients[client->getNumber()] = client;
    }
    return false;
}

void RequestsHandler::unregisterClient(std::shared_ptr<SipClient> client)
{
    std::cout << "unregister client: " << client->getNumber() << std::endl;
    _clients.erase(client->getNumber());
}

std::optional<std::shared_ptr<SipClient>> RequestsHandler::findClient(const std::string& number)
{
    auto it = _clients.find(number);
    if (it != _clients.end())
    {
        return it->second;
    }

    return {};
}

void RequestsHandler::endHandle(const std::string& destNumber, std::shared_ptr<SipMessage> message)
{
    std::cout << "[" << __FUNCTION__ << "] Sending to " << destNumber << std::endl;
    auto destClient = findClient(destNumber);
    if (destClient.has_value())
    {
        _onHandled(std::move(destClient.value()->getAddress()), std::move(message));
    }
    else
    {
        message->setHeader(SipMessageTypes::NOT_FOUND);
        auto src = message->getSource();
        _onHandled(src, std::move(message));
    }
}
