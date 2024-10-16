#include "Application.hpp"
#include "Log.hpp"
#include "SipMessageTypes.h"
#include "IDGen.hpp"
#include "AppDefines.h"

#define SIP_SERVER "192.168.0.4"
#define SIP_PORT 5060

Application::Application() : _server(SIP_SERVER, SIP_PORT, std::bind(&Application::onNewMessage, this, std::placeholders::_1, std::placeholders::_2)) {
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

void Application::OnRegister(std::shared_ptr<SipMessage> data)
{
    LOG_D << "OnRegister: " << ENDL;
}

void Application::OnCancel(std::shared_ptr<SipMessage> data)
{

}

void Application::onReqTerminated(std::shared_ptr<SipMessage> data)
{

}

void Application::OnInvite(std::shared_ptr<SipMessage> data)
{
    LOG_D << "OnInvite: " << ENDL;
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

}


void Application::OnOk(std::shared_ptr<SipMessage> data)
{
    LOG_I << ENDL;
    std::string cSqe = data->getCSeq();
    LOG_D << "cSqe: " << cSqe << ENDL;
    if (cSqe.find("REGISTER") != std::string::npos) {
        LOG_D << "Register successed" << ENDL;
    }
    else if (cSqe.find("INVITE") != std::string::npos) {
        // setCallState(data->getCallID(), Session::State::Ok);
    }
    else if (cSqe.find("BYE") != std::string::npos) {
        // setCallState(data->getCallID(), Session::State::Ok);
        // endHandle(data->getToNumber(), data);
    }
    else {
        LOG_D << "Unknown OK message" << ENDL;
    }
}

void Application::OnAck(std::shared_ptr<SipMessage> data)
{
    LOG_I << "OnAck: " << ENDL;
}

std::string Application::getAppTag()
{
    static std::string tag = IDGen::GenerateID(9);
    return tag;
}
