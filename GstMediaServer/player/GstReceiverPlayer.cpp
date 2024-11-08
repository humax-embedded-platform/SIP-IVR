#include "GstReceiverPlayer.h"
#include <gio/gio.h>
#include "Defines.h"

#define TAG "GstReceiverPlayer"

void GstReceiverPlayer::onNewSSrc (GstElement* object, guint arg0, guint arg1, gpointer user_data){
    Logger::getLogger()->info("onNewSrc arg0:{} arg1:{}", arg0, arg1);
}

GstCaps *GstReceiverPlayer::onRequestPtMap (GstElement* object, guint arg0, guint payload, gpointer user_data){
    if(payload == 104){
        GstCaps * caps = gst_caps_from_string("application/x-rtp, media=(string)AMR, encoding-params=(string)1, octet-align=(string)1, payload=(int)104");
        return caps;
    } else if(payload == 101){
        GstCaps * caps = gst_caps_from_string("application/x-rtp,media=(string)audio,clock-rate=(int)8000,encoding-name=TELEPHONE-EVENT,payload=(int)101");
        return caps;
    } else if(payload == 98){
        GstCaps * caps = gst_caps_from_string("application/x-rtp,media=(string)audio,clock-rate=(int)48000,encoding-name=TELEPHONE-EVENT,payload=(int)98");
        return caps;
    }
    return NULL;
}

void GstReceiverPlayer::onPadAdded (GstElement* object, GstPad* new_pad, gpointer user_data){
    GstReceiverPlayer * player = (GstReceiverPlayer *)user_data;

    GstPad *sinkpad;
    GstPadLinkReturn lres;

    Logger::getLogger()->info("new payload on pad: {}", GST_PAD_NAME (new_pad));
    GstCaps* caps = gst_pad_get_current_caps(new_pad);
    Logger::getLogger()->info("caps: {}", gst_caps_to_string(caps));
    gchar* caps_str = gst_caps_to_string(caps);
    GstStructure* stru = gst_caps_get_structure(caps, 0);
    gint payload = 0;
    gst_structure_get_int(stru, "payload", &payload);

    Logger::getLogger()->info("payload: {}", payload);

    if (payload == 8 /* PCMA */)  {
        // Do nothing
    } else if (payload == 101 /* TELEPHONE-EVENT */ || payload == 98)  {
        GError *err = 0;

        GstElement* rtpdtmfdepay = gst_bin_get_by_name(GST_BIN(player->_context->_pipeline), "rtpdtmfdepay");
        GstPad* sinkpad = gst_element_get_static_pad (rtpdtmfdepay, "sink");

        lres = gst_pad_link(new_pad, sinkpad);
        g_assert(lres == GST_PAD_LINK_OK);
        gst_object_unref(sinkpad);
    }
}

GstReceiverPlayer::GstReceiverPlayer() : GstBasePlayer() {

}

void GstReceiverPlayer::initPipeline()
{
    Logger::getLogger()->info("{}::Initializing pipeline", TAG);
    _context->_pipeline = gst_parse_launch(_launchCmd.c_str(), nullptr);
#if 0
    GstElement* udpsink = gst_bin_get_by_name(GST_BIN(_context->_pipeline), "udpsink");
    GSocket *udp_sink_socket;
    g_object_get (udpsink, "used-socket", &udp_sink_socket, NULL);
    gst_object_unref(udpsink);
#endif

    GstElement* udpsrc = gst_bin_get_by_name(GST_BIN(_context->_pipeline), "udpsrc");
    if (udpsrc) {
        g_object_set (udpsrc, "port", MEDIA_DEFAULT_PORT, NULL);
#if 0
        g_object_set (udpsrc, "socket", udp_sink_socket, NULL);
#endif
        gst_object_unref(udpsrc);
    }

    GstElement* rtpbin = gst_bin_get_by_name(GST_BIN(_context->_pipeline), "rtpbin");
    // GstElement* rtpptdemux = gst_bin_get_by_name(GST_BIN(pipeline), "rtpptdemux");
    g_signal_connect (rtpbin, "pad-added", G_CALLBACK (onPadAdded), this);
    g_signal_connect (rtpbin, "request-pt-map", G_CALLBACK (onRequestPtMap), NULL);
    g_signal_connect (rtpbin, "on-new-ssrc", G_CALLBACK (onNewSSrc), NULL);
    gst_object_unref(rtpbin);
}

void GstReceiverPlayer::destroyPipeline()
{
    if (_context->_pipeline) {
        gst_element_set_state(_context->_pipeline, GST_STATE_NULL);
        gst_object_unref(_context->_pipeline);
        _context->_pipeline = nullptr;
    }
}

gpointer GstReceiverPlayer::onPlayerThreadStarted(gpointer data)
{
    Logger::getLogger()->info("GstReceiverPlayer thread started");
    return nullptr;
}

gboolean GstReceiverPlayer::onBusCallback(GstBus *bus, GstMessage *message, gpointer data)
{
    GstReceiverPlayer * player = (GstReceiverPlayer *)data;
    GstElement * pipeline = player->_context->_pipeline;
    Logger::getLogger()->info("############# Got {} message.. src:{}", GST_MESSAGE_TYPE_NAME (message), gst_element_get_name (message->src));

    switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_STATE_CHANGED:
    {
        GstState old_state, new_state, pending_state;
        gst_message_parse_state_changed(message, &old_state, &new_state, &pending_state);
        Logger::getLogger()->info("state change to {} from {} with {} pending",
                     gst_element_state_get_name(new_state),
                     gst_element_state_get_name(old_state),
                     gst_element_state_get_name(pending_state));

        break;
    }
    case GST_MESSAGE_ERROR:
    {
        GError *err;
        gchar *debug;

        gst_message_parse_error (message, &err, &debug);
        Logger::getLogger()->error("########  Error: {}", err->message);
        g_error_free (err);
        g_free (debug);
        break;
    }
    case GST_MESSAGE_EOS:
    {
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
        break;
    }
    case GST_MESSAGE_ELEMENT:
    {
        const GstStructure * structure = gst_message_get_structure(message);
        auto cb = [](GQuark field, const GValue *value, gpointer user_data) -> gboolean {
            GstReceiverPlayer * player = (GstReceiverPlayer *)user_data;
            const char* fieldname = g_quark_to_string (field);
            if(strcmp(fieldname, "number") == 0){
                gchar *str = (char *)gst_value_serialize (value);
                Logger::getLogger()->info("DTMF Event: {}", str);
                player->setDTMFEvent(str);
            }
            return (gboolean)TRUE;
        };
        gst_structure_foreach(structure, cb, player);
        break;
    }
    default:
        break;
    }
    return TRUE;
}

std::string GstReceiverPlayer::getDTMFEvent()
{
    return _dtmfEvent;
}

void GstReceiverPlayer::setDTMFEvent(const std::string &dtmfEvent)
{
    _dtmfEvent = dtmfEvent;
}
