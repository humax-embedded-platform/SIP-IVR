#include "SipServer.hpp"
#include "SipMessageTypes.h"

SipServer::SipServer(std::string ip, int port) :
	_socket(ip, port, std::bind(&SipServer::onNewMessage, this, std::placeholders::_1, std::placeholders::_2)),
	_handler(ip, port, std::bind(&SipServer::onHandled, this, std::placeholders::_1, std::placeholders::_2))
{
	_socket.startReceive();
}

void SipServer::onNewMessage(std::string data, sockaddr_in src)
{
	// std::cout << "data: " << data << std::endl;
	auto message = _messagesFactory.createMessage(std::move(data), std::move(src));
	if (message.has_value())
	{
		_handler.handle(std::move(message.value()));
	}
}

void SipServer::onHandled(const sockaddr_in& dest, std::shared_ptr<SipMessage> message)
{
	// std::cout << "Sending message: " << message->toString() << std::endl;
	_socket.send(dest, message->toString());
}