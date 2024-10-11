#ifndef SDPSIPMESSAGE_H
#define SDPSIPMESSAGE_H

#include <iostream>
#include "SipMessage.h"

using namespace std;

class SipMessage;

class SdpSipMessage : public SipMessage
{
public:
    explicit SdpSipMessage(string localIp,string sipPort,string sipServer);

    string getSipString() override;
};

#endif // SDPSIPMESSAGE_H
