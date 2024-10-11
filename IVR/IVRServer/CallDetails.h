#ifndef CALLDETAILS_H_
#define CALLDETAILS_H_

#include <string>
#include <mutex>
enum callStates
{
    REGISTERED,  // State when the client registered to server.
    REGISTERING, // State when the client tries to register.
    CONNECTED,   // State when the call initiated.
    CONNECTING,  // State when the call tries to connect.
    TERMINATED,  // State when the call terminated.
    TERMINATING, // State when the call tries to terminating.
};
using namespace std;
class CallDetails
{

    typedef struct
    {
        //CallerUri
        string callerURI;
        //CalleeeUri
        string calleeURI;
        //CallId
        string callId;
        //CallStates
        callStates callState;
    } CallStruct;

public:
    //Generates call structure
    CallDetails(string callId,string callerURI,string calleURI);
    virtual ~CallDetails();
    //
    void setCallId(string callId);
    void setCallerURI(string callerUri);
    void setCalleeURI(string calleeUri);
    void setCallState(callStates state);
    callStates getCallState();
    string getCallId();
    string getCallerURI();
    string getCalleeURI();

private:
    CallStruct callStruct;
    // Call Struct parameters can be changed via receiveMessage thread and the main thread so 
    // all assignments must be safe.
    mutex readCallStructMutex;
};

#endif /* CALLDETAILS_H_ */