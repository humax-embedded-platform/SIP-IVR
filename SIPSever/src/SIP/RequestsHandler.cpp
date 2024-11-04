#include "RequestsHandler.hpp"
#include <sstream>
#include "SipMessageTypes.h"
#include "SipSdpMessage.hpp"
#include "IDGen.hpp"
#include "Log.hpp"
#include "SipMessageFactory.hpp"
#include <random>

#define SIP_SERVER_IP "192.168.0.3"

#define IVR_NAME "mvnivr"
#define IVR_IP "192.168.0.3"
#define IVR_PORT "5555"

#define CLIENT_NAME "client1"
#define CLIENT_IP "192.168.0.8"
#define CLIENT_PORT "8340"

#define AGENT_NAME "agent2"
#define AGENT_IP "192.168.0.6"

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
    response->setContact("Contact: <sip:" + data->getFromNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ">");
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
    auto caller = findClient(data->getFromNumber());
    if (caller == nullptr) {
        LOG_E << "Caller not found: " << data->getFromNumber() << ENDL;
        return;
    }

    auto called = findClient(data->getToNumber());
    if (called == nullptr) {
        // Send "SIP/2.0 404 Not Found"
        data->setHeader(SipMessageTypes::NOT_FOUND);
        data->setContact("Contact: <sip:" + caller->getNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ">");
        endHandle(data->getFromNumber(), data);
        return;
    }

    auto message = dynamic_cast<SipSdpMessage*>(data.get());
    if (!message)
    {
        LOG_E << "Couldn't get SDP from " << data->getFromNumber() << "'s INVITE request." << ENDL;
        return;
    }

    caller->setMediaDescContent(message->mediaDescContent());
    auto newSession = std::make_shared<Session>(data->getCallID(), caller, message->getRtpPort());
    _sessions.emplace(data->getCallID(), newSession);

    auto response = data;
    response->setContact("Contact: <sip:" + caller->getNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ">");
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
    LOG_I << "OnBye: " << ENDL;
    std::string callId = data->getCallID();
    auto session = getSession(callId);
    if (!session.has_value()) {
        LOG_E << "Session not found for callID: " << callId << ENDL;
        return;
    }

    std::shared_ptr<SipClient> dest =  session->get()->getDest();
    std::shared_ptr<SipClient> src = session->get()->getSrc();
    std::shared_ptr<SipClient> referedDest = session->get()->getReferedDest();

    setCallState(data->getCallID(), Session::State::Bye);
    if (!referedDest) {
        endHandle(data->getToNumber(), data);
    } else {
        if (data->getFromNumber() == src->getNumber()) {
            data->setHeader(std::string("BYE sip:") + referedDest->getNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ";transport=UDP SIP/2.0");
            data->setTo("To: <sip:" + referedDest->getNumber() + "@" + _serverIp + ">;tag=" + session.value()->getReferedToTag());
            endHandle(referedDest->getNumber(), data);
        } else if (data->getFromNumber() == referedDest->getNumber()) {
            data->setFrom("From: <sip:" + dest->getNumber() + "@" + _serverIp +">;tag=" + session.value()->getToTag());
            std::string via = data->getVia();
            // replace
            std::string partern = referedDest->getIp() + ":" + std::to_string(referedDest->getPort());
            LOG_D << "partern: " << partern << ENDL;
            if (via.find(partern) != std::string::npos) {
                via.replace(via.find(partern), partern.size(), dest->getIp() + ":" + std::to_string(dest->getPort()));
            }
            data->setVia(via);
            data->setContact("Contact: <sip:" + dest->getNumber() + "@" + _serverIp + ">");
            data->setCSeq("CSeq: 2 BYE");
            endHandle(src->getNumber(), data);
        }
    }
}

#define BUFFER_SIZE 4096

void RequestsHandler::OnRefer(std::shared_ptr<SipMessage> refer)
{
    LOG_I << ENDL;
    std::string callId = refer->getCallID();
    auto session = getSession(callId);
    if (!session.has_value()) {
        LOG_E << "Session not found for callID: " << callId << ENDL;
        return;
    }

    std::string replacedCallId = "";
    std::string replacedFromTag, replacedToTag;

    std::shared_ptr<SipClient> dest =  session->get()->getDest();
    std::shared_ptr<SipClient> src = session->get()->getSrc();
    std::shared_ptr<SipClient> referedDest = findClient(refer->getReferToNumber());

    LOG_D << "Src: " << src->getNumber() << ENDL;
    LOG_D << "Dest: " << dest->getNumber() << ENDL;
    LOG_D << "Refered to: " << referedDest->getNumber() << ENDL;

    if (dest->getNumber() != IVR_NAME) {
        LOG_E << "Refer is only allowed for IVR application." << ENDL;
        return;
    }


    SipMessageFactory factory;
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE,
             "SIP/2.0 202 Accepted\r\n"
             "Via: SIP/2.0/UDP %s:%d;branch=z9hG4bK1234\r\n"
             "From: <sip:%s@%s>\r\n"
             "To: <sip:%s@%s>\r\n"
             "Call-ID: %s\r\n"
             "CSeq: 1 REFER\r\n"
             "Content-Length: 0\r\n\r\n",
             dest->getIp().c_str(),
             dest->getPort(),
             dest->getNumber().c_str(),
             _serverIp.c_str(),
             src->getNumber().c_str(),
             _serverIp.c_str(),
             callId.c_str());
    auto responseMsg = factory.createMessage(response, session.value()->getSrc()->getAddress());
    if (!responseMsg.has_value()) {
        LOG_E << "Failed to create response message." << ENDL;
        return;
    }

    // Send a 202 Accepted response for the REFER.
    endHandle(dest->getNumber(), responseMsg.value());

#if 0
    std::string content = std::string("v=0\r\n"
                "o=- 7223043 7223302 IN IP4 " + src->getIp() +  "\r\n" +
                "s=eyeBeam\r\n"
                "c=IN IP4 " + src->getIp() + "\r\n"
                "t=0 0\r\n"
                "m=audio ") + std::to_string(session.value()->getSrcRtpPort()) + " RTP/AVP 100 6 0 8 3 18 5 101\r\n" +
                "a=alt:1 1 : 03CBAE4D 00000018 " + src->getIp() + " " + std::to_string(session.value()->getSrcRtpPort()) + "\r\n" +
                "a=fmtp:101 0-15\r\n" +
                "a=rtpmap:100 speex/16000\r\n" +
                "a=rtpmap:101 telephone-event/8000\r\n" +
                "a=sendrecv\r\n";
#else 
    std::string content = src->mediaDescContent();
#endif

    std::string invite = std::string() +
                "INVITE sip:" + referedDest->getNumber() + "@" + _serverIp + ";transport=UDP SIP/2.0\r\n" +
                "Via: SIP/2.0/UDP " + src->getIp() + ":" + std::to_string(src->getPort()) +  ";branch=" + generateBranch() + ";rport\r\n" +
                "Max-Forwards: 70\r\n" +
                "Contact: <sip:" + src->getNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ">\r\n" +
                "To: <sip:" + referedDest->getNumber() + "@" + _serverIp + ">\r\n" +
                "From: <sip:" + src->getNumber() + "@" + _serverIp + ">;tag=" + session.value()->getFromTag() + "\r\n" +
                callId + "\r\n" +
                "CSeq: 1 INVITE\r\n" +
                "Allow: INVITE, ACK, CANCEL, BYE, NOTIFY, REFER, MESSAGE, OPTIONS, INFO, SUBSCRIBE\r\n" +
                "Content-Type: application/sdp\r\n" +
                "Supported: replaces, norefersub, extended-refer, timer, sec-agree, outbound, path, X-cisco-serviceuri\r\n" +
                "User-Agent: Z 5.6.4 v2.10.20.4_1\r\n" +
                "Allow-Events: presence, kpml, talk, as-feature-event\r\n" +
                "Content-Length: " + std::to_string(content.size()) + "\r\n" +
                "\r\n" +
                content;

    // Send invite to the refer target.
    auto inviteMsg = factory.createMessage(invite, src->getAddress());
    endHandle(referedDest->getNumber(), inviteMsg.value());
    session.value()->setReferedDest(referedDest, -1);
}

void RequestsHandler::OnOk(std::shared_ptr<SipMessage> data)
{
    LOG_I << ENDL;
    auto session = getSession(data->getCallID());
    if (session.has_value())
    {
        if (session.value()->getState() == Session::State::Cancel)
        {
            endHandle(data->getFromNumber(), data);
            return;
        }

        std::shared_ptr<SipClient> src = session.value()->getSrc();
        std::shared_ptr<SipClient> dest = session.value()->getDest();
        std::shared_ptr<SipClient> referedDest = session.value()->getReferedDest();

        if (data->getCSeq().find(SipMessageTypes::INVITE) != std::string::npos)
        {
            bool transferedInvite = false, updateMediaInvite = false;

            auto client = findClient(data->getToNumber());
            if (!client) {
                LOG_E << "Client not found: " << data->getToNumber() << ENDL;
                return;
            }

            auto sdpMessage = dynamic_cast<SipSdpMessage*>(data.get());
            if (!sdpMessage)
            {
                std::cerr << "Coudn't get SDP from: " << client->getNumber() << "'s OK message.";
                endCall(data->getCallID(), data->getFromNumber(), data->getToNumber(), "SDP parse error.");
                return;
            }

            if (referedDest) {
                if (referedDest->getNumber() == sdpMessage->getToNumber()) {
                    referedDest->setMediaDescContent(sdpMessage->mediaDescContent());
                    transferedInvite = true;
                } else if (src->getNumber() == sdpMessage->getToNumber()) {
                    updateMediaInvite = true;
                }
            }

            LOG_D << "CallID: " << data->getCallID() << " transferedInvite: " << transferedInvite << " updateMediaInvite: " << updateMediaInvite << ENDL;

            std::string _to = sdpMessage->getTo();
            if (_to.find("tag=") != std::string::npos) {
                std::string toTag = _to.substr(_to.find("tag=") + 4);
                if (transferedInvite) {
                    session->get()->setReferedToTag(toTag);
                } else if (updateMediaInvite) {
                    // in case updateMediaInvite, do nothing.
                } else {
                    session->get()->setToTag(toTag);
                }
            }

            std::string _from = sdpMessage->getFrom();
            if (_from.find("tag=") !=  std::string::npos) {
                if (updateMediaInvite) {
                    // in case updateMediaInvite, do nothing
                } else {
                    std::string fromTag = _from.substr(_from.find("tag=") + 4);
                    session->get()->setFromTag(fromTag);
                }
            }

            if (!transferedInvite && !updateMediaInvite) {
                client->setMediaDescContent(sdpMessage->mediaDescContent());
                session->get()->setDest(client, sdpMessage->getRtpPort());
                session->get()->setState(Session::State::Connected);

                auto response = data;
                response->setContact("Contact: <sip:" + data->getToNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ">");
                endHandle(data->getFromNumber(), std::move(response));
            } else {
                // Send re-invite to the refer client to update media info.
                if (transferedInvite) {
                    LOG_D << "Transfered INVITE OK." << ENDL;
                    session.value()->setReferedDest(referedDest, sdpMessage->getRtpPort());
                    uint32_t port = session->get()->getReferedDestRtpPort();
#if 0
                    std::string content = std::string("v=0\r\n"
                                "o=- 179839242 179839248 IN IP4 " + referedDest->getIp() + "\r\n" +
                                "s=eyeBeam\r\n" +
                                "c=IN IP4 " + referedDest->getIp() + "\r\n" +
                                "t=0 0\r\n" +
                                "m=audio ") + std::to_string(port) + " RTP/AVP 100 6 0 8 3 18 5 101\r\n" +
                                "a=alt:1 1 : 0FF5390C 000000AF " + referedDest->getIp() + " " + std::to_string(port) + "\r\n" +
                                "a=fmtp:101 0-15\r\n" +
                                "a=rtpmap:100 speex/16000\r\n" +
                                "a=rtpmap:101 telephone-event/8000\r\n" +
                                "a=sendrecv\r\n";
#else
                    std::string content = referedDest->mediaDescContent();
#endif
                    std::string invite = std::string() +
                                "INVITE sip:" + src->getNumber() +"@" + _serverIp + ";transport=UDP SIP/2.0\r\n" +
                                "Via: SIP/2.0/UDP " + dest->getIp() + ":" + std::to_string(dest->getPort()) + ";branch=" + generateBranch() + ";rport\r\n" +
                                "Max-Forwards: 70\r\n"  +
                                "Contact: <sip:" + dest->getNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ">\r\n" +
                                "To: <sip:" + src->getNumber() + "@" + _serverIp + ">;tag=" + session.value()->getFromTag() + "\r\n" +
                                "From: <sip:" + dest->getNumber() + "@" + _serverIp + ">;tag=" + session.value()->getToTag() +"\r\n" +
                                session.value()->getCallID() + "\r\n" +
                                "CSeq: 2 INVITE\r\n" +
                                "Allow: INVITE, ACK, CANCEL, BYE, NOTIFY, REFER, MESSAGE, OPTIONS, INFO, SUBSCRIBE\r\n" +
                                "Content-Type: application/sdp\r\n" +
                                "Supported: replaces, norefersub, extended-refer, timer, sec-agree, outbound, path, X-cisco-serviceuri\r\n" +
                                "User-Agent: Z 5.6.4 v2.10.20.4_1\r\n" +
                                "Allow-Events: presence, kpml, talk, as-feature-event\r\n" +
                                "Content-Length: " + std::to_string(content.size()) + "\r\n" +
                                "\r\n" +
                                content;

                    SipMessageFactory factory;
                    auto inviteMsg = factory.createMessage(invite, session.value()->getSrc()->getAddress());
                    endHandle(src->getNumber(), inviteMsg.value());
                } else if (updateMediaInvite) {
                    LOG_D << "Update media INVITE OK. -> Send ACK to " << referedDest->getNumber() << ENDL;
                    std::string ackAgent = std::string() +
                                           "ACK sip:" + referedDest->getNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ";transport=UDP SIP/2.0\r\n" +
                                           "To: <sip:" + referedDest->getNumber() + "@" + _serverIp + ">;tag=" + session.value()->getReferedToTag() + "\r\n" +
                                           "From: <sip:" + src->getNumber() + "@" + _serverIp + ">;tag=" + session.value()->getFromTag() + "\r\n" +
                                           "Via: SIP/2.0/UDP " + src->getIp() + ":" + std::to_string(src->getPort()) + ";branch=z9hG4bK-524287-1---c4ee7dee1e5d938d;rport\r\n" +
                                           session.value()->getCallID() + "\r\n" +
                                           "CSeq: 1 ACK\r\n" +
                                           "Contact: <sip:" + src->getNumber() + "@" + src->getIp() + ":" + std::to_string(src->getPort()) + ">\r\n" +
                                           "Max-Forwards: 70\r\n" +
                                           "User-Agent: eyeBeam release 3007n stamp 17816" + "\r\n" +
                                           "Content-Length: 0\r\n" +
                                           "\r\n";
                    SipMessageFactory factory;
                    auto ackMsg = factory.createMessage(ackAgent, src->getAddress());
                    endHandle(referedDest->getNumber(), ackMsg.value());

                    // Send ACK to client
                    std::string ackClient = std::string() +
                                            "ACK sip:" + src->getNumber() + "@" + _serverIp + ":" + std::to_string(_serverPort) + ";transport=UDP SIP/2.0\r\n" +
                                            "To: <sip:" + src->getNumber() + "@" + _serverIp + ">;tag=" + session.value()->getFromTag() + "\r\n" +
                                            "From: <sip:" + dest->getNumber() + "@" + _serverIp + ">;tag=" + session.value()->getToTag() + "\r\n" +
                                            "Via: SIP/2.0/UDP " + dest->getIp() + ":" + std::to_string(dest->getPort()) + ";branch=z9hG4bK-524287-1---c4ee7dee1e5d938d;rport\r\n" +
                                            session.value()->getCallID() + "\r\n" +
                                            "CSeq: 2 ACK\r\n" +
                                            "Contact: <sip:" + dest->getNumber() + "@" + dest->getIp() + ":" + std::to_string(dest->getPort()) + ">\r\n" +
                                            "Max-Forwards: 70\r\n" +
                                            "User-Agent: eyeBeam release 3007n stamp 17816" + "\r\n" +
                                            "Content-Length: 0\r\n" +
                                            "\r\n";
                    auto ackClientMsg = factory.createMessage(ackClient, dest->getAddress());
                    endHandle(src->getNumber(), ackClientMsg.value());

                    // send BYE to the IVR
                    std::string bye = std::string() +
                                "BYE sip:" + dest->getNumber() + "@" + _serverIp + " SIP/2.0\r\n" +
                                "Via: SIP/2.0/UDP " + src->getIp() + ":" + std::to_string(src->getPort()) + ";branch=z9hG4bK-524287-1---c4ee7dee1e5d938d;rport\r\n" +
                                "Max-Forwards: 70\r\n" +
                                "To: <sip:" + dest->getNumber() + "@" + _serverIp + ">\r\n" +
                                "From: <sip:" + src->getNumber() + "@" + _serverIp + ">;tag=" + session.value()->getFromTag() + "\r\n" +
                                session.value()->getCallID() + "\r\n" +
                                "CSeq: 2 BYE\r\n" +
                                "Content-Length: 0\r\n" +
                                "\r\n";
                    auto byeMsg = factory.createMessage(bye, src->getAddress());
                    endHandle(dest->getNumber(), byeMsg.value());
                }
                return;
            }
        } else if (data->getCSeq().find(SipMessageTypes::BYE) != std::string::npos) {
            if (session.value()->getState() == Session::State::Bye)
            {
                LOG_I << "Calling terminated." << ENDL;
                if (!referedDest) {
                    endHandle(data->getFromNumber(), data);
                    endCall(data->getCallID(), data->getToNumber(), data->getFromNumber());
                } else {
                    if (data->getContactNumber() == src->getNumber()) {
                        // Send 200 OK to the refered dest.
                        data->setFrom("From: <sip:" + referedDest->getNumber() + "@" + _serverIp + ">;tag=" + session.value()->getReferedToTag());
                        //update via
                        std::string via = data->getVia();
                        std::string partern = dest->getIp() + ":" + std::to_string(dest->getPort());
                        if (via.find(partern) != std::string::npos) {
                            via.replace(via.find(partern), partern.size(), referedDest->getIp() + ":" + std::to_string(referedDest->getPort()));
                        }
                        data->setVia(via);
                        endHandle(referedDest->getNumber(), data);
                        endCall(data->getCallID(), data->getToNumber(), data->getFromNumber());
                    } else if (data->getContactNumber() == referedDest->getNumber()) {
                        // Send 200 OK to the src.
                        data->setFrom("From: <sip:" + dest->getNumber() + "@" + _serverIp + ">;tag=" + session.value()->getToTag());
                        endHandle(src->getNumber(), data);
                        endCall(data->getCallID(), data->getFromNumber(), data->getToNumber());
                    }
                }
            }
        } else {
            if (dest && dest->getNumber() == data->getFromNumber() && referedDest) {
                LOG_I << "IVR disconnected." << ENDL;
            } else {
                LOG_E << "Invalid CSeq: " << data->getCSeq() << ENDL;
            }
        }
    } else {
        LOG_E << "Session not found for callID: " << data->getCallID() << ENDL;
    }
}

void RequestsHandler::OnAck(std::shared_ptr<SipMessage> data)
{
    LOG_I << "OnAck: " << data->toString() << ENDL;
    auto session = getSession(data->getCallID());
    if (!session.has_value()) {
        LOG_E << "Session not found for callID: " << data->getCallID() << ENDL;
        return;
    }

    endHandle(data->getToNumber(), data);

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

std::shared_ptr<SipClient> RequestsHandler::findClient(const std::string& number)
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
    if (destClient) {
        _onHandled(std::move(destClient->getAddress()), std::move(message));
    }
    else
    {
        message->setHeader(SipMessageTypes::NOT_FOUND);
        auto src = message->getSource();
        _onHandled(src, std::move(message));
    }
}
