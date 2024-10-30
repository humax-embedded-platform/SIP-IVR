#include "GstSenderPlayer.h"

GstSenderPlayer::GstSenderPlayer() : GstBasePlayer() {}

GstSenderPlayer::~GstSenderPlayer()
{

}

void GstSenderPlayer::initPipeline()
{
    spdlog::info("Initializing pipeline");
    _context->_pipeline = gst_parse_launch(_launchCmd.c_str(), nullptr);

    // set pipeline to NULL state
    gst_element_set_state(_context->_pipeline, GST_STATE_NULL);
}

void GstSenderPlayer::destroyPipeline()
{
    spdlog::info("Destroying pipeline");
}

void GstSenderPlayer::setLaunchCmd(const std::string &launchCmd)
{
    _launchCmd = launchCmd;
}

void GstSenderPlayer::setPBSourceFile(const std::string &sourceFile)
{
    if (_context->_pipeline) {
        GstElement *filesrc = gst_bin_get_by_name(GST_BIN(_context->_pipeline), "source");
        if (!filesrc) {
            spdlog::error("Failed to get filesrc element");
            return;
        }
        g_object_set(G_OBJECT(filesrc), "location", sourceFile.c_str(), nullptr);
        gst_object_unref(filesrc);
    }
}

gpointer GstSenderPlayer::onPlayerThreadStarted(gpointer data)
{
    spdlog::info("Player thread started");
    return nullptr;
}

gboolean GstSenderPlayer::onBusCallback(GstBus *bus, GstMessage *message, gpointer data)
{
    GstSenderPlayer * player = (GstSenderPlayer *)data;
    GstElement * pipeline = player->_context->_pipeline;
    spdlog::info("############# Got {} message.. src:{}", GST_MESSAGE_TYPE_NAME (message), gst_element_get_name (message->src));

    switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_STATE_CHANGED:
    {
        break;
        GstState old_state, new_state, pending_state;
        gst_message_parse_state_changed(message, &old_state, &new_state, &pending_state);
        spdlog::info("state change to {} from {} with {} pending",
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
        spdlog::error("########  Error: {}", err->message);
        g_error_free (err);
        g_free (debug);
        break;
    }
    case GST_MESSAGE_EOS:
    {
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
        break;
    }
    case GST_MESSAGE_ELEMENT: //gstdtmfdemay event..
    {
        spdlog::info("dtmf event detected");
        const GstStructure * structure = gst_message_get_structure(message);

        auto cb = [](GQuark field, const GValue *value, gpointer user_data) -> gboolean {
            // GstPlayer * player = (GstPlayer *)user_data;
            // GstElement * pipeline = core->pipeline;


            // gchar *str = (char *)gst_value_serialize (value);
            // const char* fieldname = g_quark_to_string (field);
            // if(strcmp(fieldname, "number") == 0){
            //     auto response = web::json::value::object();
            //     response["tool_inf"]["inf_type"] = web::json::value::string("event");
            //     response["tool_inf"]["tool_id"] = web::json::value::number(core->tool_id);
            //     response["tool_inf"]["data"]["type"] = web::json::value::string("RTP_event_detected");
            //     response["tool_inf"]["data"]["event"] = web::json::value::string(std::string{"DTMF"} + std::string{str});

            //     auto rsp = response.serialize();
            //     uint32_t rsp_len = rsp.length();
            //     std::vector<boost::asio::const_buffer> buffers;
            //     buffers.push_back(boost::asio::buffer(&rsp_len, sizeof(rsp_len)));
            //     buffers.push_back(boost::asio::buffer(rsp, rsp_len));

            //     boost::asio::async_write(core->sock, buffers,
            //                              [](const boost::system::error_code & error, size_t len){
            //                                  if(error){
            //                                      return;
            //                                  }
            //                              });

            // }
            // g_print ("%15s: %s\n", fieldname, str);
            // g_free (str);
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
