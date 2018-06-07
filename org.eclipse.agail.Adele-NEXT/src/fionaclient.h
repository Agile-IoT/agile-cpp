#ifndef __NEXT_FIONACLIENT_H__
#define __NEXT_FIONACLIENT_H__

#include <string>
//#include <vector>
//#include <chrono>

// Protocol used in the communication with the Service Manager and with the Virtual Robot (except for the audio)
enum Protocol {
	undefined,
	http,
	https,
};

// Establishes a connection with a Fiona Virtual Robot, allowing the communication through audio or chat
class FionaClient {
public:
	bool isConnected () {return inited;}	// returns true if the connection is established
	void setInAudioEnabled (bool inAudioEnabled);	// enables/disables the input audio thread
	void setOutAudioEnabled (bool outAudioEnabled);	// enables/disables the output audio thread
	int setAvatar (std::string usermail, std::string usrid2, std::string avatarId, std::string avname);	// sets the avatar identification parameters
	int setProtocol (Protocol protocol);	// sets the protocol (HTTP, HTTPS...)
	int setSm (std::string smUrl);	// sets the SM
	int setAio (std::string aioUrl);	// sets the AIO
	int setDefAio (std::string defAioUrl);	// sets the default AIO (in case the SM does not respond)
	int setRtmpServer (std::string rtmpServerUrl);	// sets the RTMP server
	int initConnection ();	// initializes the connection with the VR
	int endConnection ();	// ends the connection with the VR

private:
	bool inited = false;
	bool inAudioEnabled = true;
	bool outAudioEnabled = true;
	bool avatarSet = false;
	std::string usermail;
	std::string usrid2;
	std::string avatarId;
	std::string avname;
	std::string protocol;
	std::string defProtocol = "http:";
	std::string smUrl;
	std::string defSmUrl = "sm.adelerobots.com";
	std::string aioUrl;
	std::string defAioUrl;
	std::string defDefAioUrl = "aio0001.adelerobots.com";
	std::string rtmpServerUrl;
	std::string defRtmpServerUrl = "195.55.126.59";

	void *vrSession;
	std::string roomId;
	void *sigCheckTimer;
	void *recMsgPollTimer;
	void *inAudioThread;
	void *outAudioThread;
	void setUninitToDefault ();
	int requestAio ();
	int initializeVr ();
	int setCookies ();
	int sendSigCheck ();
	void initInAudioThread ();
	void initOutAudioThread ();
	std::string generateRandomRoomId (int length);
};

#endif
