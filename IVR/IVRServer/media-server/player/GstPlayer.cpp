#include "GstPlayer.h"
#include <gst/gst.h>
#include <spdlog/spdlog.h>
#include "util/FileUtil.h"

static bool initialized = false;

GstPlayer::GstPlayer() :
    _rtpPort(0),
    _rtpHost(""),
    _codec(""),
    _payloadType(0),
    _sampleRate(0),
    _pbSourceFile(""),
    _sender_pipeline(nullptr),
    _receiver_pipeline(nullptr) {
    if (!initialized) {
#if 0
        g_setenv("GST_DEBUG", "4", 1);
#endif
        gst_init(nullptr, nullptr);
        initialized = true;
    }
}

void GstPlayer::setRtpPort(int port)
{
    _rtpPort = port;
}

void GstPlayer::setRtpHost(std::string host)
{
    _rtpHost = host;
}

void GstPlayer::setCodec(std::string codec)
{
    spdlog::info("Setting codec: {}", codec);
    _codec = codec;
}

void GstPlayer::setPayloadType(int payloadType)
{
    spdlog::info("Setting payload type: {}", payloadType);
    _payloadType = payloadType;
}

void GstPlayer::setSampleRate(int sampleRate)
{
    spdlog::info("Setting sample rate: {}", sampleRate);
    _sampleRate = sampleRate;
}

void GstPlayer::setPBSourceFile(std::string sourceFile)
{
    if (sourceFile.empty()) {
        spdlog::error("Source file is empty");
        return;
    }

    auto absPath = FileUtil::absolutePath(sourceFile);
    spdlog::info("Setting playback source file: {}", absPath);

    _pbSourceFile = absPath;

    if (_sender_pipeline) {
        GstElement *filesrc = gst_bin_get_by_name(GST_BIN(_sender_pipeline), "source");
        if (!filesrc) {
            spdlog::error("Failed to get filesrc element");
            return;
        }
        g_object_set(G_OBJECT(filesrc), "location", _pbSourceFile.c_str(), nullptr);
        gst_object_unref(filesrc);

        GstState state;
        gst_element_get_state(_sender_pipeline, &state, nullptr, GST_CLOCK_TIME_NONE);
        if (state == GST_STATE_PLAYING) {
            spdlog::info("Resetting pipeline ...");
            gst_element_set_state(_sender_pipeline, GST_STATE_READY);
        }
    }
}

bool GstPlayer::open()
{
    if (_rtpPort == 0 || _rtpHost.empty()) {
        spdlog::error("RTP host or port not set");
        return false;
    }

    initPipeline();
    return true;
}

bool GstPlayer::start()
{
    spdlog::info("Starting player ...");
    if (_sender_pipeline) {
        gst_element_set_state(_sender_pipeline, GST_STATE_PLAYING);
    }
    return true;
}

bool GstPlayer::stop()
{
    if (_sender_pipeline) {
        gst_element_set_state(_sender_pipeline, GST_STATE_NULL);
    }
    return true;
}

bool GstPlayer::close()
{
    if (_sender_pipeline) {
        gst_element_set_state(_sender_pipeline, GST_STATE_NULL);
        gst_object_unref(_sender_pipeline);
    }

    if (_receiver_pipeline) {
        gst_element_set_state(_receiver_pipeline, GST_STATE_NULL);
        gst_object_unref(_receiver_pipeline);
    }
    return true;
}

void GstPlayer::initPipeline()
{
    spdlog::info("Initializing pipeline ...");
    if (!_sender_pipeline) {
        std::string cmd;

        // check parameters is missing
        if (_codec.empty() || _payloadType == 0 || _sampleRate == 0) {
            spdlog::error("Missing parameters");
            return;
        }

        if (_codec == "opus") {
            cmd = std::string("filesrc name=source") + _pbSourceFile + " ! wavparse ! audioconvert ! opusenc ! rtpopuspay pt=" +
                  std::to_string(_payloadType) + " ! udpsink host=" + _rtpHost + " port=" + std::to_string(_rtpPort);
        } else if (_codec == "speex") {
            cmd = std::string("filesrc name=source") + _pbSourceFile + " ! wavparse ! audioconvert ! audioresample ! audio/x-raw,rate=" + std::to_string(_sampleRate) + " ! speexenc ! rtpspeexpay pt=" +
                  std::to_string(_payloadType) + " ! udpsink host=" + _rtpHost + " port=" + std::to_string(_rtpPort);
        } else {
            spdlog::error("Unsupported codec");
            return;
        }

        spdlog::info("Gstreamer command: {}", cmd);

        _sender_pipeline = gst_parse_launch(cmd.c_str(), nullptr);
        g_assert(_sender_pipeline);

        GstBus * bus = gst_pipeline_get_bus (GST_PIPELINE (_sender_pipeline));
        if(bus != nullptr){
            gst_bus_add_watch (bus, GstPlayer::busCallback, this);
            gst_object_unref (bus);
        }

        // set the pipeline to paused
        GstStateChangeReturn ret = gst_element_set_state(_sender_pipeline, GST_STATE_NULL);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            spdlog::error("Unable to set the pipeline to the null state");
            return;
        }
    }
}

gboolean GstPlayer::busCallback(GstBus *bus, GstMessage *message, gpointer data) {
    GstPlayer * player = (GstPlayer *)data;
    GstElement * pipeline = player->_sender_pipeline;
    auto bus_callback_id = std::this_thread::get_id();
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
        // if(core->current_file_index + 1 == core->filelist.size()){
        //     core->GstClear();
        // }
        // else {
        //     gst_element_set_state(pipeline, GST_STATE_READY);
        //     (*core).current_file_index += 1;
        //     GstElement * filesrc = gst_bin_get_by_name(GST_BIN(pipeline), "filesrc");
        //     g_object_set (filesrc, "location", core->filelist[core->current_file_index].c_str(), NULL);

        //     GstElement * rtpamrpay = gst_bin_get_by_name(GST_BIN(pipeline), "rtppay");

        //     GstRTPBasePayload *rtpbasepayload = GST_RTP_BASE_PAYLOAD (rtpamrpay);
        //     guint32 new_timestamp = rtpbasepayload->timestamp;
        //     guint16 new_seq = rtpbasepayload->seqnum;
        //     // TODO AMR은 160으로 gstreamer에 하드코딩 되어있음.  https://github.com/GStreamer/gst-plugins-good/blob/master/gst/rtp/gstrtpamrpay.c, line:348
        //     // TODO 각 코덱 별로 값을 찾아 정의해줘야 할듯..
        //     g_object_set(rtpamrpay, "timestamp-offset", new_timestamp + 160, "seqnum-offset", new_seq + 1, NULL);

            gst_element_set_state(pipeline, GST_STATE_PLAYING);
        // }
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
        /* unhandled message */
        break;
    }
    return TRUE;
}



GstPlayer::~GstPlayer()
{
    close();
}
