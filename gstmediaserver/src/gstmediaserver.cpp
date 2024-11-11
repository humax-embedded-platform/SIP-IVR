#include "gstmediaserver.h"
#include "MediaServer.h"

static gstmediaserver::MediaServer* s_media_server = nullptr;
void gstmediaserver::start()
{
    if (!s_media_server) {
        s_media_server = new gstmediaserver::MediaServer();
        s_media_server->startServer();
    }
}
