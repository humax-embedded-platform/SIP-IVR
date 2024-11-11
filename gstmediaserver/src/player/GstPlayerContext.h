#ifndef GSTPLAYERCONTEXT_H
#define GSTPLAYERCONTEXT_H


#include <gst/gst.h>

namespace gstmediaserver {

class GstPlayerContext
{
public:
    explicit GstPlayerContext();
    virtual ~GstPlayerContext();

public:
    GstElement* _pipeline;
    GThread* _thread;
    GMainLoop* _loop;
    GMainContext* _context;
    GstBus* _bus;
    GMutex _lock;
    GCond _cond;
};

} // namespace gstmediaserver

#endif // GSTPLAYERCONTEXT_H
