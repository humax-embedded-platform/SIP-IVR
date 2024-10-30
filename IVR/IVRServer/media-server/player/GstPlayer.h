#ifndef GSTPLAYER_H
#define GSTPLAYER_H

#include <string>
#include <gst/gst.h>
#include "GstPlayerContext.h"

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

private:
    void initPipeline();

    static gboolean busCallback(GstBus * bus, GstMessage * message, gpointer data);

private:
    int _rtpPort;
    std::string _rtpHost;
    std::string _codec;
    int _payloadType;
    int _sampleRate;
    std::string _pbSourceFile;

    GThread* _senderThread;
    GstElement* _sender_pipeline;

    GThread* _receiverThread;
    GstElement* _receiver_pipeline;
};

#endif // GSTPLAYER_H
