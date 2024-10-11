#ifndef SIPMESSAGE_H_
#define SIPMESSAGE_H_


#include <cstring>
#include <iostream>
#include <string>
#include <queue>
#include <unistd.h>
#include <sstream>

#define CRLF "\r\n"

#define SIP_VERSION "SIP/2.0"
#define TRANSPORT_PROTOCOL "SIP/2.0/UDP" //Transport protocol that will be used in 'via' header.

#define REGISTER "REGISTER"
#define INVITE "INVITE"
#define BYE "BYE"

#define CONTENT_LENGTH_HEADER "Content-Length"
#define CALLID_HEADER "Call-ID"

using namespace std;

class SipMessage {
    typedef struct {

            string method; // Method of the request.
            string via; // Request
            string statusCode; //Response

            //Headers
            string from;
            string to; 
            string callId; // Random id to follow the sip messaging.
            string cSeq; // Sequence number. 
            string contact; 
            string body;    // Body, bypass for now.
            string contentType;
            int contentLength = 0; // Size of the body.
        } Message;
    public:
        //Constructor is initiated with 
        //Params:
        // localIp: ip address or hostname of the local machine.(ccpenv for us).
        // sipPort; port address for sip messageing.(Defaukt 5060 for us). 
        // sipServer: ip address or hostname of the sip server.(mockasterisk for us).
        SipMessage(string localIp,string sipPort,string sipServer);
        SipMessage();//Creates empty sip struct for responses.
        
        virtual ~SipMessage();//Creates an empty sip message.

        //Sets method 
        void setMethod(string type, string calleeURI);

        //sets Via header by using:
        // defined TRANSPORT_PROTOCOL, 
        // LocalIp parameter which is preset in the constructor.
        // SipPort parameters which is preset in the constructor.
        void setVia();
        //sets "From" parameter of the sipMessage struct. Parameters:
        //callerURI: the sip uri of the caller.
        void setFrom(string callerUri);//Display name can be added.
        //sets "To" parameter of the sipMessage struct. 
        //Parameters:
        //calleUri: the sip uri of the callee.
        void setTo(string calleeUri);//Display name can be added.
        //sets "callId" of the sipMessage struct.
        //Parameters:
        //callId: Specific id for the call.
        void setCallId(string callId);
        //sets "cSeq"parameters of the sipMesasge struct.
        //Parameters:
        //seq: sequence number.
        void setCseq(int seq);
        //sets "contact" parameter of the sipMessage struct.
        //Parameters:
        //callerUri: Sip uri of the caller.
        void setContact(string callerUri);

        // set "contentLength" parameter of the sipMessage struct.
        // Parameters:
        // length: length of the body.
        void setContentLength(int length);

        // set "content type" parameter of the sipMessage struct.
        // Parameters:
        // type: type of the content.
        void setContentType(string type);

        // Returns callId of the sipMessage struct.
        string getCallId();
        // Returns the "statuscode" of the sipMessage struct.
        string getStatus();
        // Returns the "contentLength of the sipMessage struct."
        int getContentLength();
        // Creates a sip message string by using the sipMessage struct.
        virtual string getSipString();
        // Generate sipMessage struct by the given sipString parameter.
        //Parameters:
        // sipString: the string message which will be generated to sipMessage.
        void setSipFromString(string sipString);
        
    protected:
        Message sipMessage;
        string localIp;
        string sipPort;
        string sipServer;
        // Parses the first line of the sip string to get the "statusCode".
        void parseStartLine(string line);


};

#endif /* SIPMESSAGE_H_ */
