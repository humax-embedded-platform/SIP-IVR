#include "GstPlayerContext.h"

GstPlayerContext::GstPlayerContext() :
    _pipeline(nullptr),
    _thread(nullptr),
    _loop(nullptr),
    _context(nullptr)
{
    g_mutex_init(&_lock);
    g_cond_init(&_cond);
}

GstPlayerContext::~GstPlayerContext()
{
    g_mutex_clear(&_lock);
    g_cond_clear(&_cond);
}