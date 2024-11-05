# Project structure

## 1. SIPServer
Implmenting a SIP proxy which will handle all request from client, IVR application and agent

## 2. GstMediaServer
Implment a media server using Gstreamer, it's responsible to play some audio file as a auto respond bot, beside, media also decode DTMF event from client and respond to IVR application

## 3. IVR 
  ### 3.1 IVRGST
  An application will handle request media server to play some audio files as an automated answers to client and request redirect call to agent corresponding the DTMF key which client pressed.

  ### 3.1 IVRPJSIP
  Another IVR application which using PJSIP library, it's just a simple app for testing.
