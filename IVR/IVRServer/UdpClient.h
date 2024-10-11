#ifndef UDPCLIENT_H_
#define UDPCLIENT_H_
#include <cstring>
#include <iostream>
#include <string>
#include <queue>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <netdb.h>


using namespace std;
class UdpClient {
    private:
    public:
        //Constructor
        UdpClient(string serverAddress,int port);
        //Deconstructor
        virtual ~UdpClient();
        //Send message To udp client.
        //Pushes the message to sendQueue queue.
        void pushMessageToSendQ(string message);
        //Get message from udp Client.
        //Pops message from recvQ .
        string popMessageFromRecvQ();
    private:
        // UDP socket.
        int sockfd;
        struct sockaddr_in serverAddr;
        //Queueing the messages for sending to socket.
        queue<string> sendQueue;
        //Queueing the messages from receiving from socket.
        queue<string> recvQueue;
        mutex sendMutex,recvMutex;
        //Sentthread is used for getting data from sendQueue and sending to socket.
        thread sendThread;
        //Receive Thread is for getting data from socket and pushing it to RecvQueue.
        thread recvThread;
        //pops data from sendQueue and sends it to socket. In our case it is a sipMessage.
        //Running as thread.
        void processSendQueue();
        //Used for stopping the threads safely.
        atomic_bool running;
        //gets data from socket and pushes it to recvQueue. 
        //Running as thread.
        void processReceiveQueue();
        //Pushes message to RecvQueue.
        void pushMessageToRecvQ(string message);

};

#endif /* UDPCLIENT_H_ */