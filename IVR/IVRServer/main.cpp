#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "cxxopts.hpp"
#include "Application.hpp"
#include "SipMessage.hpp"
#include "CallDetails.h"
#include "SipSdpMessage.hpp"
#include <iostream>
#include <sstream>
#include <thread>

#define SIP_SERVER "192.168.0.4"
#define LOCAL_IP "192.168.0.4"
#define SIP_PORT "5060"
#define CALLER_URI "sip:phong4@192.168.0.4"
#define CALLEE_URI "sip:phong2@192.168.0.4"

/*
thread receiveSipThread;

// Function prototypes
// Pushes sip messages to UdpClient.sendQueue
void sendSIPMessage(SipMessage &message);
// Process incoming sip messages and changes status of the calldetails.
void receiveSIPMessage(SipMessage message, CallDetails *callDetails);

void registerUser(CallDetails *callDetails);
void initiateCall(CallDetails *callDetails);
void terminateCall(CallDetails *callDetails);
void parseIncomingMessage(CallDetails *callDetails);

// For stopping the receiveSipThread safely.
atomic_bool running = true;
*/

int main(int argc, char** argv)
{


    cxxopts::Options options("IVR Application", "Open source application for an automated telephone system");

    options.add_options()
        ("h,help", "Print usage")
        ("i,ip", "Sip server ip", cxxopts::value<std::string>())
        ("p,port", "Sip server ip (default: 5060).", cxxopts::value<int>()->default_value(std::to_string(5060)));

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    try
    {
        std::string ip = result["ip"].as<std::string>();
        int port = result["port"].as<int>();
        Application app;
        std::cout << "Application has been started ..." << std::endl;
        getchar();
    }
    catch (const cxxopts::OptionException&)
    {
        std::cout << "Please enter ip and port." << std::endl;
    }
}

/*
void parseIncomingMessage(CallDetails *callDetails)
{
    string sipMessageString;

    size_t startPos = 0;
    string delimiter = "\r\n\r\n";
    string buffer = "";
    while (running)
    {

        string buffer = udp->popMessageFromRecvQ();
        std::cout << "buffer: " << buffer << std::endl;



        // sleep(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

// Implement the functions declared above
void registerUser(CallDetails *callDetails)
{

    SipMessage sipMessage(LOCAL_IP, SIP_PORT, SIP_SERVER);
    sipMessage.setMethod(REGISTER, callDetails->getCallerURI());
    sipMessage.setFrom(callDetails->getCallerURI());
    sipMessage.setTo(callDetails->getCallerURI());
    sipMessage.setCallId(callDetails->getCallId());
    sipMessage.setContact(callDetails->getCallerURI());

    callDetails->setCallState(REGISTERING);

    sendSIPMessage(sipMessage);

    // TODO: Construct and send SIP REGISTER message
    // Omitting detailed message construction for candidate to implement
}
void initiateCall(CallDetails *callDetails)
{
    SdpSipMessage sipMessage(LOCAL_IP, SIP_PORT, SIP_SERVER);
    sipMessage.setMethod(INVITE, callDetails->getCalleeURI());
    sipMessage.setFrom(callDetails->getCallerURI());
    sipMessage.setTo(callDetails->getCalleeURI());
    sipMessage.setCallId(callDetails->getCallId());
    sipMessage.setContact(callDetails->getCallerURI());
    sipMessage.setContentType("application/sdp");

    callDetails->setCallState(CONNECTING);

    std::cout << "msg: " << sipMessage.getSipString() << std::endl;
    sendSIPMessage(sipMessage);

    // TODO: Implement call initiation logic
    // Hint: Start with an INVITE message and handle the response
}
void terminateCall(CallDetails *callDetails)
{

    SipMessage sipMessage(LOCAL_IP, SIP_PORT, SIP_SERVER);
    sipMessage.setMethod(BYE, callDetails->getCalleeURI());
    sipMessage.setFrom(callDetails->getCallerURI());
    sipMessage.setTo(callDetails->getCalleeURI());
    sipMessage.setCallId(callDetails->getCallId());
    sipMessage.setContact(callDetails->getCallerURI());

    callDetails->setCallState(TERMINATING);

    sendSIPMessage(sipMessage);

    // TODO: Implement call termination logic
    // Hint: Send a BYE message and handle the response
}
// Utility functions for sending and receiving SIP messages
// NOTE: Simplify the network communication aspect for this case study
void sendSIPMessage(SipMessage &message)
{
    udp->pushMessageToSendQ(message.getSipString());
    // Simulated message sending (details omitted)
}

void receiveSIPMessage(SipMessage message, CallDetails *callDetails)
{

    if (message.getCallId() == callDetails->getCallId())
    { // Control if the incoming sip message's related to our call.
        string callStatus = message.getStatus();

        if (callStatus == "200")
        {
            switch (callDetails->getCallState())
            {
            case REGISTERING:
                callDetails->setCallState(REGISTERED);
                printf("REGISTERED.\n");
                break;
            case CONNECTING:
                callDetails->setCallState(CONNECTED);
                printf("CONNECTED.\n");
                break;
            case TERMINATING:
                callDetails->setCallState(TERMINATED);
                printf("TERMINATED.\n");
                break;
            default:
                break;
            }
        }
        if (callStatus == "100")
        {
            printf("TRYING.\n");
        }
        else if (callStatus == "180")
        {
            printf("RINGING.\n");
        }
        else if (callStatus == "400")
        {
            printf("BAD REQUEST.\n");
        }
        else if (callStatus == "401")
        {
            printf("UNAUTHORIZED.\n");
        }
        else if (callStatus == "404")
        {
            printf("NOT FOUND.\n");
        }
        else if (callStatus == "408")
        {
            printf("REQUEST TIMEOUT.\n");
        }
        else if (callStatus == "500")
        {
            printf("SERVER INTERNAL ERROR.\n");
        }
        else if (callStatus == "503")
        {
            printf("SERVICE UNAVAILABLE.\n");
        }
        else if (callStatus == "600")
        {
            printf("BUSY EVERYWHERE.\n");
        }
        else if (callStatus == "603")
        {
            printf("DECLINED.\n");
        }
        else if (callStatus == "604")
        {
            printf("DOES NOT EXIST.\n");
        }
    }
    // Simulated message receiving (details omitted)
    // The candidate needs to figure out how to handle different responses
}
*/
