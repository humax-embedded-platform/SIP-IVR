#ifndef GSTPLAYER_H
#define GSTPLAYER_H


#include <string>
#include <gst/gst.h>
#include "GstPlayerContext.h"
#include <memory>

namespace gstmediaserver {

class GstSenderPlayer;
class GstReceiverPlayer;
class GstPlayer
{
public:
    GstPlayer();
    ~GstPlayer();

    void setRtpPort(int port);
    void setRtpHost(std::string host);
    void setCodec(std::string codec);
    void setPayloadType(int payloadType);
    void setSampleRate(int sampleRate);
    void setPBSourceFile(std::string sourceFile);

    bool open();
    bool start();
    bool stop();
    bool close();

    std::string getDTMFEvent();
    void clearDTMFEvent();

private:
    void initPlayer();


private:
    int _rtpPort;
    std::string _rtpHost;
    std::string _codec;
    int _payloadType;
    int _sampleRate;
    std::string _pbSourceFile;

    std::shared_ptr<GstSenderPlayer> _senderPlayer;
    std::shared_ptr<GstReceiverPlayer> _receiverPlayer;
};
} // namespace gstmediaserver


#endif // GSTPLAYER_H
