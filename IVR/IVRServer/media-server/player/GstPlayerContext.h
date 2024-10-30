#ifndef GSTPLAYERCONTEXT_H
#define GSTPLAYERCONTEXT_H

#include <gst/gst.h>

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
    GMutex _lock;
    GCond _cond;
};

#endif // GSTPLAYERCONTEXT_H
