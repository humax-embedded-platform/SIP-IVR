#include "RequestHandler.h"
#include "spdlog/spdlog.h"
#include "json.hpp"

using json = nlohmann::json;

RequestHandler::RequestHandler() {}

void RequestHandler::handleRequest(int conectionFd, const char *buffer, int len)
{
    spdlog::info("Handling request ...{}", buffer);
    json req = json::parse(buffer);
    if (req.contains(REQUEST_TYPE) && req.contains(REQUEST_DATA)) {
        std::string requestType = req[REQUEST_TYPE];
        json data = req[REQUEST_DATA];
        spdlog::info("Data: {}", data.dump());
        if (requestType == REQUEST_TYPE_INIT_SESSION) {
            spdlog::info("Handling init_session request ...");
        }
    }
}
