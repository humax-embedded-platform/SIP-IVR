#ifndef DTMFHANDLER_H
#define DTMFHANDLER_H

#include "MediaSession.h"

class DTMFHandler : public MediaSessionCallback
{
public:
    DTMFHandler();

    void onDTMFEvent(std::shared_ptr<MediaSession> session, std::string event) override;
};

#endif // DTMFHANDLER_H
