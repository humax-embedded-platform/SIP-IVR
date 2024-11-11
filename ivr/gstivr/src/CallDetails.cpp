#include "CallDetails.h"

namespace gstivr {

CallDetails::CallDetails(string callId,string callerURI,string calleURI){
    setCallId(callId);
    setCallerURI(callerURI);
    setCalleeURI(calleURI);

}
 CallDetails::~CallDetails(){
    
 }
void CallDetails::setCallId(string callId)
{
    lock_guard<mutex> lock(readCallStructMutex);
    callStruct.callId = callId;
}

void CallDetails::setCallerURI(string callerUri)
{

    lock_guard<mutex> lock(readCallStructMutex);
    callStruct.callerURI = callerUri;
}
void CallDetails::setCalleeURI(string calleeUri)
{
    lock_guard<mutex> lock(readCallStructMutex);
    callStruct.calleeURI = calleeUri;
}
void CallDetails::setCallState(callStates state)
{
    lock_guard<mutex> lock(readCallStructMutex);
    callStruct.callState = state;
}
callStates CallDetails::getCallState()
{
    lock_guard<mutex> lock(readCallStructMutex);
    return callStruct.callState;
}
string CallDetails::getCallId()
{
    lock_guard<mutex> lock(readCallStructMutex);
    return callStruct.callId;
}
string CallDetails::getCallerURI()
{
    lock_guard<mutex> lock(readCallStructMutex);
    return callStruct.callerURI;
}
string CallDetails::getCalleeURI()
{
    lock_guard<mutex> lock(readCallStructMutex);
    return callStruct.calleeURI;
}
} // namespace gstivr
