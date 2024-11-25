![Project Logo](images/logo.png)
<div align="center">
  <img src="https://www.3cx.com/wp-content/uploads/2020/06/ivr-system.jpg" alt="Logo" width="200">
  <p>This is a description below the centered logo.</p>
</div>

# I. Project structure

## 1. SIPServer
This modules will handle all SIP/SDP messages from/to clients, IVR application, and agents, such as: REGISTER, INVITE, BYE, REFER, ACK...  
It’s responsible to initialize, manage the sessions, client and transport the messages (SIP/SDP) between clients via UDP sockets.

## 2. GstMediaServer
The main responsibility of server is playback the audio script to the clients base on request from IVR application, and listen DTMF event from clients and return back to IVR application. 
Media server will make 2 players: one for sending audio data via RTP and one for receiving audio data via RTP. The players are basically gstreamer pipelines.

## 3. IVR 
  ### 3.1 IVRGST
  Tt exists as a SIP client, which can response a call: accept (200 OK), cancel (CANCEL), hang up (BYE) ... 
  However, the different is it can communicate to the media server for playing audio script, which user can listen as a automatically guidance. 
  Beside, IVR application can listen to event from media server corresponding DTMF key event, then request SIP server to redirect (REFER) the call to agents. 
  The communication between IVR, SIP Server and Media Server all via UDP sockets.

  ### 3.1 IVRPJSIP
  Another IVR application which using PJSIP library, it's just a simple app for testing.


# II. Build and Run

## 1. Prerequisites 
  - gstreamer 1.16+
  - openssl
  - cmake 3.16+

## 2. Build and Run
### 2.1 Download source code
```bash
git clone git@github.com:humax-embedded-platform/SIP-IVR.git
```

### 2.2 Build and run using scipt
```bash
cd SIP-IVR
source launch.sh -d -m /home/phongdang/WorkSpace/SipServer/blob
```
#### Usage
-d : build with debug mode  
-m : your directory that store audio scripts

### 2.2 Build and run by yourself

``` bash
cd SIP-IVR
mkdir -p build
cd build
cmake ..
make
./sip-ivr -i 192.168.0.3 -m /home/phongdang/WorkSpace/SipServer/blob # correct you media path
```

#### Usage
-i : your server IP address  
-m : your directory that store audio scripts

# III. Constraints
This project tested and supported Zoiper Softphone only, with other one, we hasn't tested yet.
Supported media codecs:
  - opus/48000
  - speex/16000
