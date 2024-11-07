#ifndef GSTSENDERPLAYER_H
#define GSTSENDERPLAYER_H

#include "GstBasePlayer.h"
#include <gst/gst.h>

typedef struct FileContext {
    std::string file_path;
    long file_size = 0;
    long offset = 0;
    FileContext() : file_path(""), file_size(0), offset(0) {}
} FileContext;

class GstSenderPlayer : public GstBasePlayer
{
public:
    explicit GstSenderPlayer(std::string host, int port);
    ~GstSenderPlayer();

    void initPipeline() override;
    void destroyPipeline() override;
    void setPBSourceFile(const std::string &sourceFile);

    gpointer onPlayerThreadStarted(gpointer data) override;
    gboolean onBusCallback(GstBus *bus, GstMessage *message, gpointer data) override;

private:
    std::string _host;
    int _port;
    FileContext _fileContext;
};

#endif // GSTSENDERPLAYER_H
