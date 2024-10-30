#include "GstBasePlayer.h"

GstBasePlayer::GstBasePlayer() {
    _isOpened = false;
    _context = std::make_shared<GstPlayerContext>();
}

bool GstBasePlayer::open()
{
    _isOpened = true;
    MUTEX_LOCK(&_context->_lock);

    if (!_context->_thread) {
        _context->_thread = g_thread_new("player-thread", onBasePlayerThreadStarted, this);
        while (!_context->_loop || !g_main_loop_is_running(_context->_loop))
            g_cond_wait(&_context->_cond, &_context->_lock);
    }

    MUTEX_UNLOCK(&_context->_lock);
    return true;
}

bool GstBasePlayer::start()
{
    INVOKE_WITH_MAIN_CONTEXT_DEFAULT(_context->_context, playOnMainCtx, this);
    return true;
}

bool GstBasePlayer::stop()
{
    INVOKE_WITH_MAIN_CONTEXT_DEFAULT(_context->_context, stopOnMainCtx, this);
    return true;
}

bool GstBasePlayer::close()
{
    if (!isOpened()) {
        return false;
    }

    INVOKE_WITH_MAIN_CONTEXT_DEFAULT(_context->_context, closeOnMainCtx, this);
    while (_context->_loop && g_main_loop_is_running(_context->_loop)) {
        spdlog::info("Waiting for player thread to stop ...");
        g_usleep(1000000); //sleep for 1 second
    }
    _isOpened = false;
    return true;
}

bool GstBasePlayer::isOpened()
{
    return _isOpened;
}

gpointer GstBasePlayer::onBasePlayerThreadStarted(gpointer data)
{
    GstBasePlayer* player = (GstBasePlayer*)data;
    player->_context->_context = g_main_context_new();
    player->_context->_loop = g_main_loop_new(player->_context->_context, FALSE);

    g_main_context_push_thread_default(player->_context->_context);

    player->initPipeline();
    player->_context->_bus = gst_pipeline_get_bus (GST_PIPELINE (player->_context->_pipeline));
    if(player->_context->_bus != nullptr){
        gst_bus_add_watch (player->_context->_bus, busCallback, player);
    }


    GSource* source = g_idle_source_new();
    g_source_set_callback(source, (GSourceFunc)onMainLoopStarted, player, NULL);
    g_source_attach(source, player->_context->_context);
    g_source_unref(source);

    player->onPlayerThreadStarted(data);

    spdlog::info("Starting main loop ...");
    g_main_loop_run(player->_context->_loop);
    spdlog::info("Main loop stopped");

    MUTEX_LOCK(&player->_context->_lock);
    while (g_main_context_iteration(player->_context->_context, FALSE)) {}
    g_main_context_pop_thread_default(player->_context->_context);
    MUTEX_UNLOCK(&player->_context->_lock);

    if (player->_context->_bus) {
        gst_bus_remove_watch(player->_context->_bus);
    }

    g_main_loop_unref(player->_context->_loop);
    player->_context->_loop = nullptr;

    g_main_context_unref(player->_context->_context);
    player->_context->_context = nullptr;

    spdlog::info("Player thread stopped");
    g_thread_exit(NULL);
    return NULL;
}

gboolean GstBasePlayer::onMainLoopStarted(gpointer data)
{
    GstBasePlayer* player = (GstBasePlayer*)data;
    MUTEX_LOCK(&player->_context->_lock);
    g_cond_signal(&player->_context->_cond);
    MUTEX_UNLOCK(&player->_context->_lock);
    spdlog::info("Main loop started");
    return G_SOURCE_REMOVE;
}

gboolean GstBasePlayer::playOnMainCtx(gpointer data)
{
    spdlog::info("Playing ...");
    GstBasePlayer* player = (GstBasePlayer*)data;
    // change state to playing
    gst_element_set_state(player->_context->_pipeline, GST_STATE_PLAYING);
    return G_SOURCE_REMOVE;
}

gboolean GstBasePlayer::stopOnMainCtx(gpointer data)
{
    spdlog::info("Stopping ...");
    GstBasePlayer* player = (GstBasePlayer*)data;
    // change state to null
    gst_element_set_state(player->_context->_pipeline, GST_STATE_NULL);
    return G_SOURCE_REMOVE;
}

gboolean GstBasePlayer::closeOnMainCtx(gpointer data)
{
    spdlog::info("Closing ...");
    GstBasePlayer* player = (GstBasePlayer*)data;
    if (!player->isOpened()) {
        spdlog::error("Player is not opened");
        return G_SOURCE_REMOVE;
    }

    if (player->_context->_loop && g_main_loop_is_running(player->_context->_loop)) {
        g_main_loop_quit(player->_context->_loop);
    }
    return G_SOURCE_REMOVE;
}

gboolean GstBasePlayer::busCallback(GstBus *bus, GstMessage *message, gpointer data)
{
    GstBasePlayer* player = (GstBasePlayer*)data;
    return player->onBusCallback(bus, message, data);
}
