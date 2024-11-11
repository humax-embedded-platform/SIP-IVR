#include "GstSenderPlayer.h"
#include <atomic>
#include <fstream>
#include <glib.h>
#include <gst/gst.h>
#include <stdlib.h>
#include "util/Log.hpp"

namespace gstmediaserver {

#define TAG "GstSenderPlayer"

std::atomic<bool> file_changed(false);


void need_data_callback(GstElement *appsrc, guint length, gpointer user_data) {
    FileContext &fileContext = *(FileContext *)user_data;

    // If file_changed is true, close the current file and open the new file
    if (file_changed.load()) {
        Logger::getLogger()->info("File changed, opening new file: {}", fileContext.file_path);
        fileContext.file_size = 0;
        fileContext.offset = 0;
        file_changed = false;
    }

    FILE *file;
    unsigned char buffer[length];
    size_t bytes_read;

    file = fopen(fileContext.file_path.c_str(), "rb");
    if (file == NULL) {
        Logger::getLogger()->error("Failed to open file: {}", fileContext.file_path);
        return;
    }

    if (!fileContext.file_size) {
        fseek(file, 0, SEEK_END);
        fileContext.file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
    }

    if (fileContext.offset >= fileContext.file_size) {
        Logger::getLogger()->info("End of file reached -> replaying from the beginning");
        fileContext.offset = 0;
    }

    // Move the file pointer to the specified offset
    if (fseek(file, fileContext.offset, SEEK_SET) != 0) {
        perror("Error seeking file");
        fclose(file);
        return;
    }


    memset(buffer, 0, length);
    if ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        // Logger::getLogger()->info("Read {} bytes from file from offset {} in total {} bytes", bytes_read, fileContext.offset, fileContext.file_size);
        GstBuffer *gst_buffer = gst_buffer_new_allocate(NULL, bytes_read, NULL);
        gst_buffer_fill(gst_buffer, 0, buffer, bytes_read);

        GstFlowReturn ret;
        g_signal_emit_by_name(appsrc, "push-buffer", gst_buffer, &ret);
        gst_buffer_unref(gst_buffer);

        if (ret != GST_FLOW_OK) {
            Logger::getLogger()->error("Error pushing buffer to appsrc!");
        }

        fileContext.offset += bytes_read;
    }
    fclose(file);
}

GstSenderPlayer::GstSenderPlayer(std::string host, int port) : GstBasePlayer() {
    _host = host;
    _port = port;
}

GstSenderPlayer::~GstSenderPlayer()
{

}

void GstSenderPlayer::initPipeline()
{
    if (_context->_pipeline) {
        Logger::getLogger()->error("{}::Pipeline already initialized", TAG);
        return;
    }

    Logger::getLogger()->info("{}::Initializing pipeline", TAG);

    if (!_launchCmd.empty()) {
        _context->_pipeline = gst_parse_launch(_launchCmd.c_str(), NULL);
    } else {
        _context->_pipeline = gst_pipeline_new("audio-pipeline");
        GstElement *appsrc = gst_element_factory_make("appsrc", "audio-source");
        GstElement *audioconvert = gst_element_factory_make("audioconvert", "convert");
        GstElement *audioresample = gst_element_factory_make("audioresample", "resample");
        GstElement *opusenc = gst_element_factory_make("opusenc", "encoder");
        GstElement *rtpopuspay = gst_element_factory_make("rtpopuspay", "rtp-payload");
        GstElement *udpsink = gst_element_factory_make("udpsink", "udp-sink");

         if (!_context->_pipeline || !appsrc || !audioconvert || !audioresample || !opusenc || !rtpopuspay || !udpsink) {
             Logger::getLogger()->error("{}::Not all elements could be created.", TAG);
            return;
        }

        // Configure appsrc
        g_object_set(G_OBJECT(appsrc),
                     "format", GST_FORMAT_TIME,
                     "is-live", TRUE,
                     "block", TRUE,
                     NULL);

        GstCaps *caps = gst_caps_new_simple("audio/x-raw",
                                            "format", G_TYPE_STRING, "S16LE",
                                            "rate", G_TYPE_INT, 24000,
                                            "channels", G_TYPE_INT, 1,
                                            "channel-mask", GST_TYPE_BITMASK, 0x4,
                                            "layout", G_TYPE_STRING, "interleaved",
                                            NULL);
        g_object_set(appsrc, "caps", caps, NULL);
        gst_caps_unref(caps);

        // Configure udpsink
        g_object_set(G_OBJECT(udpsink),
                     "host", _host.c_str(),
                     "port", _port,
                     "async", FALSE,
                     NULL);

        // set payload type
        g_object_set(G_OBJECT(rtpopuspay), "pt", 106, NULL);

        // Build the pipeline
        gst_bin_add_many(GST_BIN(_context->_pipeline), appsrc, audioconvert, audioresample, opusenc, rtpopuspay, udpsink, NULL);
        if (!gst_element_link_many(appsrc, audioconvert, audioresample, opusenc, rtpopuspay, udpsink, NULL)) {
            Logger::getLogger()->error("{}::Elements could not be linked.", TAG);
            gst_object_unref(_context->_pipeline);
            return;
        }

        // Set need-data signal handler for appsrc
        g_signal_connect(appsrc, "need-data", G_CALLBACK(need_data_callback), &_fileContext);
    }

    // set pipeline to NULL state
    gst_element_set_state(_context->_pipeline, GST_STATE_NULL);
}

void GstSenderPlayer::destroyPipeline()
{
    Logger::getLogger()->info("Destroying pipeline");
    if (_context->_pipeline) {
        gst_element_set_state(_context->_pipeline, GST_STATE_NULL);
        gst_object_unref(_context->_pipeline);
        _context->_pipeline = nullptr;
    }
}

void GstSenderPlayer::setPBSourceFile(const std::string &sourceFile)
{
    if (!sourceFile.empty()) {
        Logger::getLogger()->info("Setting source file: {}", sourceFile);

        GstElement *filesrc = gst_bin_get_by_name(GST_BIN(_context->_pipeline), "filesrc");
        if (filesrc) {
            // case use filesrc
            gst_element_set_state(_context->_pipeline, GST_STATE_NULL);
            g_object_set(G_OBJECT(filesrc), "location", sourceFile.c_str(), nullptr);
            gst_element_set_state(_context->_pipeline, GST_STATE_PLAYING);
            gst_object_unref(filesrc);
        } else {
            // case use appsrc
            if (!_fileContext.file_path.empty() && sourceFile != _fileContext.file_path ) {
                file_changed = true;
            }
            _fileContext.file_path = sourceFile;
            _fileContext.file_size = 0;
            _fileContext.offset = 0;
        }
    }
}

gpointer GstSenderPlayer::onPlayerThreadStarted(gpointer data)
{
    Logger::getLogger()->info("GstSenderPlayer thread started");
    return nullptr;
}

gboolean GstSenderPlayer::onBusCallback(GstBus *bus, GstMessage *message, gpointer data)
{
    GstSenderPlayer * player = (GstSenderPlayer *)data;
    GstElement * pipeline = player->_context->_pipeline;
    Logger::getLogger()->info("############# Got {} message.. src:{}", GST_MESSAGE_TYPE_NAME (message), gst_element_get_name (message->src));

    switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_STATE_CHANGED:
    {
        GstState old_state, new_state, pending_state;
        gst_message_parse_state_changed(message, &old_state, &new_state, &pending_state);
        Logger::getLogger()->info("{}'s state change to {} from {} with {} pending",
                     gst_element_get_name (message->src),
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
        g_usleep(5000000);
        if (!gst_element_seek(pipeline, 1.0, GST_FORMAT_TIME,
                              (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE),
                              GST_SEEK_TYPE_SET, 0,
                              GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
            Logger::getLogger()->error("Failed to seek to start");
        }
        break;
    }
    default:
        break;
    }
    return TRUE;
}
} // namespace gstmediaserver

