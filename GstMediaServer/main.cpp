#include <gst/gst.h>
#include <iostream>
#include "MediaServer.h"
#include "spdlog/spdlog.h"

int main(int argc, char *argv[]) {
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    MediaServer server(DEFAULT_PORT);
    return 0;
}
