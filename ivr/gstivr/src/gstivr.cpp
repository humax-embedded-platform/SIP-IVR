#include "gstivr.h"
#include "Application.hpp"

static gstivr::Application* s_ivr_app = nullptr;
void gstivr::start(std::string server_ip, int server_port, std::string app_ip)
{
    if (!s_ivr_app) {
        s_ivr_app = new gstivr::Application(server_ip, server_port, app_ip);
    }
}
