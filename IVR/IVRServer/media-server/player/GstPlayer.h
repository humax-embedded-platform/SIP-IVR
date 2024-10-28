#ifndef GSTPLAYER_H
#define GSTPLAYER_H

#include <string>
#include <gst/gst.h>

class GstPlayer
{
public:
    GstPlayer();

    void setRtpPort(int port);
    void setRtpHost(std::string host);

    void open();
    void start();
    void stop();
    void close();

private:
    void initPipeline();

private:
    int _rtpPort;
    std::string _rtpHost;

    GstElement* _pipeline;
};

#endif // GSTPLAYER_H
