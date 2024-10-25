#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <map>

#define MAX_SESSIONS 5
class SessionManager
{
public:
    SessionManager();

    bool exceedsMaxSessions();

private:

};

#endif // SESSIONMANAGER_H
