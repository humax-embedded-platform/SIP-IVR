#ifndef GSTIVR_H
#define GSTIVR_H

#include <string>

namespace gstivr {

void  __attribute__((visibility("default"))) start(std::string server_ip, int server_port, std::string app_ip, int app_rtp_port);

} // namespace gstivr

#endif // GSTIVR_H
