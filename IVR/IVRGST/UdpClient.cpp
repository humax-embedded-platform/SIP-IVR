#include "UdpClient.hpp"
#include <sstream>
#include <vector>
#include <thread>
#include <cstring>
#include <Log.hpp>

UdpClient::UdpClient(std::string ip, int port, OnNewMessageEvent event) : _ip(std::move(ip)), _port(port), _onNewMessageEvent(event), _keepRunning(false)
{

    Logger::getLogger()->info("UdpClient: {}:{}" , _ip, _port);
#if defined _WIN32 || defined _WIN64
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cerr << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		exit(EXIT_FAILURE);
	}
#endif

    if ((_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        Logger::getLogger()->error("socket creation failed");
		exit(EXIT_FAILURE);
	}

	std::memset(&_servaddr, 0, sizeof(_servaddr));
	_servaddr.sin_family = AF_INET;
	_servaddr.sin_addr.s_addr = inet_addr(_ip.c_str());
	_servaddr.sin_port = htons(port);
}

UdpClient::~UdpClient()
{
	closeServer();
}

std::string UdpClient::getIp() const
{
    return _ip;
}

int UdpClient::getPort() const
{
    return _port;
}

void UdpClient::startReceive()
{
	_keepRunning = true;
	_receiverThread = std::thread([=]()
		{
			char buffer[BUFFER_SIZE];
			sockaddr_in senderEndPoint;
			std::memset(&senderEndPoint, 0, sizeof(senderEndPoint));
			int len = sizeof(senderEndPoint);

			while (_keepRunning)
			{
				memset(buffer, 0, BUFFER_SIZE);
				std::memset(&senderEndPoint, 0, sizeof(senderEndPoint));
#ifdef __linux__
				recvfrom(_sockfd, buffer, BUFFER_SIZE, 0, reinterpret_cast<struct sockaddr*>(&senderEndPoint), (socklen_t*)&len);
#elif defined _WIN32 || defined _WIN64
				recvfrom(_sockfd, buffer, BUFFER_SIZE, 0, reinterpret_cast<struct sockaddr*>(&senderEndPoint), &len);
#endif
				if (len) {
                    Logger::getLogger()->info("Received from {}:{} with message: \n{}" , inet_ntoa(senderEndPoint.sin_addr), ntohs(senderEndPoint.sin_port), buffer);
                }
				if (!_keepRunning) return;
				_onNewMessageEvent(std::move(buffer), senderEndPoint);
			}
		});
}

int UdpClient::send(std::string buffer)
{
    Logger::getLogger()->info("Sending to server with message: {}" , buffer);
	return sendto(_sockfd, buffer.c_str(), std::strlen(buffer.c_str()),
        0, reinterpret_cast<const struct sockaddr*>(&_servaddr), sizeof(_servaddr));
}

void UdpClient::closeServer()
{
	_keepRunning = false;
	shutdown(_sockfd, 2);
#ifdef __linux__
	close(_sockfd);
#elif defined _WIN32 || defined _WIN64
	closesocket(_sockfd);
#endif
	_receiverThread.join();
}
