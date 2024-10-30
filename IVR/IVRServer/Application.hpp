#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <UdpClient.hpp>
#include <SipMessage.hpp>
#include <SipMessageFactory.hpp>
#include "DTMFHandler.h"

class Application
{
public:
    Application(std::string server_ip, int server_port, std::string app_ip, int app_port);

private:
    void onNewMessage(std::string data, sockaddr_in src);
    void sendToServer(std::shared_ptr<SipMessage> message);

private:
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
    std::string _server_ip;
    int _server_port;
    std::string _app_ip;
    int _app_port;
    SipMessageFactory _messagesFactory;
    std::thread _registerThread;
    std::shared_ptr<DTMFHandler> _dtmfHandler;
    std::unordered_map<std::string, std::function<void(std::shared_ptr<SipMessage> request)>> _handlers;

};

#endif // APPLICATION_HPP
