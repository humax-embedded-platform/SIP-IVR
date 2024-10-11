#include "SdpSipMessage.h"


SdpSipMessage::SdpSipMessage(string localIp, string sipPort, string sipServer) : SipMessage(localIp, sipPort, sipServer)
{

}

string SdpSipMessage::getSipString()
{
    std::cout << "SdpSipMessage::" << __FUNCTION__ << std::endl;
    string msg = string("v=0") + CRLF
                "o=Z 0 20528078 IN IP4 " + localIp + CRLF
                "s=Z" + CRLF
                "c=IN IP4 " + localIp + CRLF
                "t=0 0" + CRLF
                "m=audio 42534 RTP/AVP 106 9 98 101 0 8 3" + CRLF
                "a=rtpmap:106 opus/48000/2" + CRLF
                "a=fmtp:106 sprop-maxcapturerate=16000; minptime=20; useinbandfec=1" + CRLF
                "a=rtpmap:98 telephone-event/48000" + CRLF
                "a=fmtp:98 0-16" + CRLF
                "a=rtpmap:101 telephone-event/8000" + CRLF
                "a=fmtp:101 0-16" + CRLF
                "a=sendrecv" + CRLF
                "a=rtcp-mux" + CRLF;

    setContentLength(msg.size());

    string header = SipMessage::getSipString();
    return header + msg;
}
