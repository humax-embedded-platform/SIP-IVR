#ifndef DTMFHANDLER_H
#define DTMFHANDLER_H

#include <memory>
#include "media/MediaSession.h"

namespace gstivr {

class CallSession;
class Application;
class MediaSession;

class DTMFHandler : public gstivr::MediaSessionCallback
{
public:
    explicit DTMFHandler(Application* app);

    enum KEYCODE {
        KEY_0 = 0,
        KEY_1,
        KEY_2,
        KEY_3,
        KEY_4,
        KEY_5,
        KEY_6,
        KEY_7,
        KEY_8,
        KEY_9,
        KEY_UNKNOWN
    };

    void onDTMFEvent(std::shared_ptr<gstivr::MediaSession> session, std::string event) override;

private:
    void makeRefer(std::string agentID, std::shared_ptr<CallSession> session);
    void replayGuide(std::shared_ptr<CallSession> session);
    void invalidChoose(std::shared_ptr<CallSession> session);

private:
    Application* _app;
};

} // namespace gstivr


#endif // DTMFHANDLER_H
