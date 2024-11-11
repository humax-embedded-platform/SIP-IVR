#ifndef SIPSERVER_H
#define SIPSERVER_H

#include <string>

namespace sipserver {

void __attribute__((visibility("default"))) start(std::string host, int port);

} // namespace sipserver
#endif // SIPSERVER_H
