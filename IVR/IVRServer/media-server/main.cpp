#include <gst/gst.h>
#include <iostream>

int main(int argc, char *argv[]) {
    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create the elements
    GstElement *pipeline = gst_pipeline_new("audio-pipeline");
    GstElement *source = gst_element_factory_make("autoaudiosrc", "audio-source");
    GstElement *encoder = gst_element_factory_make("opusenc", "encoder");
    GstElement *payloader = gst_element_factory_make("rtpopuspay", "payloader");
    GstElement *sink = gst_element_factory_make("udpsink", "udp-sink");

    if (!pipeline || !source || !encoder || !payloader || !sink) {
        std::cerr << "Failed to create GStreamer elements." << std::endl;
        return -1;
    }

    // Set properties
    g_object_set(sink, "host", "127.0.0.1", "port", 5000, nullptr);

    // Build the pipeline: autoaudiosrc ! opusenc ! rtpopuspay ! udpsink
    gst_bin_add_many(GST_BIN(pipeline), source, encoder, payloader, sink, nullptr);
    if (!gst_element_link_many(source, encoder, payloader, sink, nullptr)) {
        std::cerr << "Failed to link elements." << std::endl;
        gst_object_unref(pipeline);
        return -1;
    }

    // Start playing the pipeline
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "Unable to set the pipeline to the playing state." << std::endl;
        gst_object_unref(pipeline);
        return -1;
    }

    std::cout << "Streaming RTP to 127.0.0.1:5000. Press Ctrl+C to stop." << std::endl;

    // Wait until error or EOS (End of Stream)
    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg;
    bool terminate = false;
    while (!terminate) {
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                         static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

        if (msg != nullptr) {
            GError *err;
            gchar *debug_info;
            switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                std::cerr << "Error received from element " << GST_OBJECT_NAME(msg->src)
                          << ": " << err->message << std::endl;
                std::cerr << "Debugging information: " << (debug_info ? debug_info : "none") << std::endl;
                g_clear_error(&err);
                g_free(debug_info);
                terminate = true;
                break;
            case GST_MESSAGE_EOS:
                std::cout << "End-Of-Stream reached." << std::endl;
                terminate = true;
                break;
            default:
                break;
            }
            gst_message_unref(msg);
        }
    }

    // Free resources
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}
