#include "SipServer.hpp"
#include "SipMessageTypes.h"
#include "UdpServer.hpp"
#include "Log.hpp"
#include "sipserver.h"

namespace sipserver {

void start(std::string host, int port) {
    static SipServer* s_sip_server = nullptr;
    if (!s_sip_server) {
        s_sip_server = new SipServer(host, port);
    }
}

SipServer::SipServer(std::string ip, int port) :
	_socket(ip, port, std::bind(&SipServer::onNewMessage, this, std::placeholders::_1, std::placeholders::_2)),
	_handler(ip, port, std::bind(&SipServer::onHandled, this, std::placeholders::_1, std::placeholders::_2))
{
	_socket.startReceive();
    Logger::getLogger()->info("SipServer started");
}

void SipServer::onNewMessage(std::string data, sockaddr_in src)
{
    Logger::getLogger()->info("New message from {}:{}" , inet_ntoa(src.sin_addr), ntohs(src.sin_port));
	auto message = _messagesFactory.createMessage(std::move(data), std::move(src));
	if (message.has_value())
	{
		_handler.handle(std::move(message.value()));
	}
}

void SipServer::onHandled(const sockaddr_in& dest, std::shared_ptr<SipMessage> message)
{
	_socket.send(dest, message->toString());
}

} // namespace sipserver

