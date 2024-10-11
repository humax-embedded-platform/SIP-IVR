#include "UdpClient.h"

UdpClient::UdpClient(string serverAddress,int port) : running(true)
{

    struct addrinfo hints,*res;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;

    if(int status= getaddrinfo(serverAddress.c_str(),to_string(port).c_str(),&hints,&res)!=0){
        printf("Error getting address\n");
        exit(EXIT_FAILURE);
    }


    sockfd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if(sockfd<0){
        printf("Error opening socket");
        exit(EXIT_FAILURE);
    }


    memcpy(&serverAddr,res->ai_addr,res->ai_addrlen);
    
    freeaddrinfo(res);


    sendThread= thread(&UdpClient::processSendQueue,this);
    recvThread=thread(&UdpClient::processReceiveQueue,this);

}


UdpClient::~UdpClient(){
    running=false;
    shutdown(sockfd,SHUT_RDWR);
    //wait untill the thread finishes his job.
    if(sendThread.joinable()){
        sendThread.join();
    }
    close(sockfd);
}

void UdpClient::pushMessageToSendQ(string message){
    lock_guard<mutex>lock(sendMutex);
    sendQueue.push(message);
}
void UdpClient::pushMessageToRecvQ(string message){
    lock_guard<mutex>lock(recvMutex);
    recvQueue.push(message);
}
string UdpClient::popMessageFromRecvQ(){
    lock_guard<mutex>lock(recvMutex);
    string outgoingMessage;
    if(!recvQueue.empty()){
        outgoingMessage=recvQueue.front();
        recvQueue.pop();
    }
    return outgoingMessage;
}
void UdpClient::processSendQueue(){
    //Check if the program is stopped.
    while(running){
        string outgoingMessage;
        //mutex area
        {
            lock_guard<mutex>lock(sendMutex);
            if(!sendQueue.empty()){
                outgoingMessage=sendQueue.front();
                sendQueue.pop();
            }
        }
        if(!outgoingMessage.empty()){
            sendto(sockfd,outgoingMessage.c_str(),outgoingMessage.length(),0,(const struct sockaddr*)&serverAddr,sizeof(serverAddr));
            
        }else{
            //Sleep for a short duration to prevent busy waiting.
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
}
void UdpClient::processReceiveQueue(){
    char buffer[1024];
    while(running){
        socklen_t len=sizeof(serverAddr);
        int n=recvfrom(sockfd,buffer,1024,0,(struct sockaddr*)&serverAddr,&len);
        if(n>0){
            buffer[n]='\0';
            pushMessageToRecvQ(string(buffer));
        }
    }
}