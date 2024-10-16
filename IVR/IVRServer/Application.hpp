#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <UdpClient.hpp>
#include <SipMessage.hpp>
#include <SipMessageFactory.hpp>

class Application
{
public:
    Application();

private:
    void onNewMessage(std::string data, sockaddr_in src);
    void sendToServer(std::shared_ptr<SipMessage> message);

private:
    void OnRegister(std::shared_ptr<SipMessage> data);
    void OnCancel(std::shared_ptr<SipMessage> data);
    void onReqTerminated(std::shared_ptr<SipMessage> data);
    void OnInvite(std::shared_ptr<SipMessage> data);
    void OnTrying(std::shared_ptr<SipMessage> data);
    void OnRinging(std::shared_ptr<SipMessage> data);
    void OnBusy(std::shared_ptr<SipMessage> data);
    void OnUnavailable(std::shared_ptr<SipMessage> data);
    void OnBye(std::shared_ptr<SipMessage> data);
    void OnOk(std::shared_ptr<SipMessage> data);
    void OnAck(std::shared_ptr<SipMessage> data);

    std::string getAppTag();

private:
    UdpClient _server;
    SipMessageFactory _messagesFactory;
    std::thread _registerThread;
    std::unordered_map<std::string, std::function<void(std::shared_ptr<SipMessage> request)>> _handlers;

};

#endif // APPLICATION_HPP
