#include "MediaServer.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "spdlog/spdlog.h"
#include "RequestHandler.h"

MediaServer::MediaServer(int port) : _serverPort(port)
{
    spdlog::info("Creating server ...");
    std::thread t(&MediaServer::startServer, this);
    t.join();
}

void MediaServer::startServer()
{
    spdlog::info("Starting server ...");
    int server_fd = _serverFd;
    int port = _serverPort;

    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        spdlog::error("Socket failed");
        return;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        spdlog::error("setsockopt failed");
        close(server_fd);
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(DEFAULT_HOST);
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        spdlog::error("Bind failed");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 10) < 0) {
        spdlog::error("Listen failed");
        close(server_fd);
        return;
    }

    spdlog::info("Server is listening on port {} ...", port);

    int new_socket = -1;
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            spdlog::error("Accept failed");
            close(server_fd);
            return;
        }

        spdlog::info("Connection accepted!");

        std::thread client_thread(&MediaServer::handleConnection, this, new_socket);
        client_thread.detach();
    }

    close(server_fd);
}

void MediaServer::handleConnection(int clientFd)
{
    char buffer[1024] = {0};
    int valRead = read(clientFd, buffer, 1024);
    spdlog::info("Received: {}", buffer);

    _requestHandler.handleRequest(clientFd, buffer, valRead);

    // Close the client socket
    close(clientFd);
}
