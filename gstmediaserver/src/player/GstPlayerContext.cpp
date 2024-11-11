#include "GstPlayerContext.h"

namespace gstmediaserver {

GstPlayerContext::GstPlayerContext() :
    _pipeline(nullptr),
    _thread(nullptr),
    _loop(nullptr),
    _context(nullptr),
    _bus(nullptr)
{
    g_mutex_init(&_lock);
    g_cond_init(&_cond);
}

GstPlayerContext::~GstPlayerContext()
{
    g_mutex_clear(&_lock);
    g_cond_clear(&_cond);
}
} // namespace gstmediaserver
