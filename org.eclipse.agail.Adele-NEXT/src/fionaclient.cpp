#include "fionaclient.h"

#include <iostream> // TODO: substitute by log library

#include <random>
#include <thread>
#include <chrono>
#include <functional>
//#include <list>
#include <sstream>
#include <algorithm> // std::find_if

#include "httpcookiesession.h"
#include "inaudiostream.h"
#include "outaudiostream.h"

// Timer thread that every interval executes the function passed as parameter
class Timer {
	std::thread th;
	std::function<int(void)> funct;
	std::chrono::milliseconds interval;
	bool running = true;
	void threadFunct();
public:
	Timer (std::function<int(void)> funct, std::chrono::milliseconds interval) : funct(funct), interval(interval) {th = std::thread (&Timer::threadFunct, this);} // constructor
	void stop() {running = false; th.join();} // ends the thread
};

// Timer constructor: initializes the thread that calls the function every interval
void Timer::threadFunct() {
	std::chrono::high_resolution_clock::time_point next_time =
			std::chrono::high_resolution_clock::now() + interval;
	std::this_thread::sleep_until (next_time);
	while (running){
		if (funct()) { /*handle error*/ };
		next_time = next_time + interval;
		std::this_thread::sleep_until (next_time);
	}
}

// Enables/disables the input audio thread
void FionaClient::setInAudioEnabled (bool inAudioEnabled) {
	if (!inited) // Virtual Robot not initialized
		this->inAudioEnabled = inAudioEnabled;
	else if (this->inAudioEnabled == inAudioEnabled) {} // no change
	else if (inAudioEnabled) { // open input audio thread
		initInAudioThread();
		this->inAudioEnabled = true;
	}
	else { // close input audio thread
		((InAudioStream*)inAudioThread)->stop();
		this->inAudioEnabled = false;
	}
}

// Enables/disables the output audio thread
void FionaClient::setOutAudioEnabled (bool outAudioEnabled) {
	if (!inited) // Virtual Robot not initialized
		this->outAudioEnabled = outAudioEnabled;
	else if (this->outAudioEnabled == outAudioEnabled) {} // no change
	else if (outAudioEnabled){ // open output audio thread
		initOutAudioThread();
		this->outAudioEnabled = true;
	}
	else { // close output audio thread
		((OutAudioStream*)outAudioThread)->stop();
		this->outAudioEnabled = false;
	}
}

// Sets the avatar identification parameters
int FionaClient::setAvatar (std::string usermail, std::string usrid2, std::string avatarId, std::string avname) {
	if (inited) {
		std::cerr << "Virtual Robot already initialized. Cannot modify parameters" << std::endl;
		return 1;
	}
	this->usermail = usermail;
	this->usrid2 = usrid2;
	this->avatarId = avatarId;
	this->avname = avname;

	avatarSet = true;
	return 0;
}

// Sets the protocol (HTTP, HTTPS...)
int FionaClient::setProtocol (Protocol protocol){
	if (inited) {
		std::cerr << "Virtual Robot already initialized. Cannot modify protocol" << std::endl;
		return 1;
	}
	switch (protocol) {
	case http:
		this->protocol = "http:";
		break;
	case https:
		this->protocol = "https:";
		break;
	default:
		std::cerr << "Unknown protocol" << std::endl;
		break;
	}

	return 0;
}

// Sets the Service Manager
int FionaClient::setSm (std::string smUrl) {
	if (inited) {
		std::cerr << "Virtual Robot already initialized. Cannot modify Service Manager" << std::endl;
		return 1;
	}
	this->smUrl = smUrl;

	return 0;
}

// Sets the AIO
int FionaClient::setAio (std::string aioUrl) {
	if (inited) {
		std::cerr << "Virtual Robot already initialized. Cannot modify AIO" << std::endl;
		return 1;
	}
	this->aioUrl = aioUrl;

	return 0;
}

// Sets the default AIO (in case the Service Manager does not respond)
int FionaClient::setDefAio (std::string defAioUrl) {
	if (inited) {
		std::cerr << "Virtual Robot already initialized. Cannot modify default AIO" << std::endl;
		return 1;
	}
	this->defAioUrl = defAioUrl;

	return 0;
}

// Sets the RTMP server
int FionaClient::setRtmpServer (std::string rtmpServerUrl) {
	if (inited) {
		std::cerr << "Virtual Robot already initialized. Cannot modify RTMP server" << std::endl;
		return 1;
	}
	this->rtmpServerUrl = rtmpServerUrl;

	return 0;
}

// Initializes the connection with the Virtual Robot
int FionaClient::initConnection () {
	if (!avatarSet) {
		std::cerr << "Set the avatar parameters before initializing the VR" << std::endl;
		return 1;
	}
	if (inited) {
		std::cerr << "Virtual Robot already initialized" << std::endl;
		return 1;
	}

	// Set uninitialized URLs to their default values
	setUninitToDefault();

	// Set the AIO
	if (aioUrl.empty()){
		if (requestAio()) // ask SM for the AIO
			return 1;
	}
	std::cerr << "AIO URL: " << aioUrl << std::endl;

	// Initialize VR
	if (initializeVr())
		return 1;

	// Set cookies
	if (setCookies())
		return 1;

	// Initialize sigCheck timer
	sigCheckTimer = new Timer(std::bind(&FionaClient::sendSigCheck, this),std::chrono::milliseconds (5000));

	// Wait for some time (to avoid sigCheck and message polling requests being sent together)
	std::this_thread::sleep_for(std::chrono::milliseconds (100));

	// Initialize the thread that reads the default input audio device and writes it on the output stream
	if (inAudioEnabled)
		initInAudioThread();

	// Initialize the thread that plays the input audio stream in the default output device
	if (outAudioEnabled)
		initOutAudioThread();

	inited = true;
	return 0;
}

// Sets the uninitialized URLs to their default values
void FionaClient::setUninitToDefault () {
	// Set the protocol (HTTP, HTTPS...)
	if (protocol.empty()) {
		std::cerr << "No protocol defined. Setting \"" << defProtocol << "\" as protocol" << std::endl;
		protocol = defProtocol;
	}

	// Set the default AIO URL (to be used in case the Service Manager doesn't answer with any AIO URL)
	if (aioUrl.empty() && defAioUrl.empty()) {
		std::cerr << "No AIO or default AIO set. Setting \"" << defDefAioUrl << "\" as default AIO" << std::endl;
		defAioUrl = defDefAioUrl;
	}

	// Set the Service Manager
	if (aioUrl.empty() && smUrl.empty()) {
		std::cerr << "No Service Manager set. Setting \"" << defSmUrl << "\" as Service Manager" << std::endl;
		smUrl = defSmUrl;
	}

	// Set the RTMP server
	if (rtmpServerUrl.empty()) {
		std::cerr << "No RTMP server set. Setting \"" << defRtmpServerUrl << "\" as RTMP server" << std::endl;
		rtmpServerUrl = defRtmpServerUrl;
	}
}

// Ends the connection with the Virtual Robot
int FionaClient::endConnection () {
	if (!inited) {
		std::cerr << "Virtual Robot not initialized yet. Cannot end connection" << std::endl;
		return 1;
	}

	// End threads
	if (inAudioEnabled)
		((InAudioStream*)inAudioThread)->stop();
	if (outAudioEnabled)
		((OutAudioStream*)outAudioThread)->stop();
	((Timer*)sigCheckTimer)->stop();

	// Send end connection message to the VR
	std::vector<HttpParam> endConnparams;
	endConnparams.push_back (HttpParam{"action","endConnection"});
	std::cerr << "Ending connection with the Virtual Robot..." << std::endl;
	HttpResponse endConnResponse = ((HttpCookieSession*)vrSession)->performHttpGet (endConnparams);
	if (endConnResponse.code != HTTP_SUCCESS) {
		std::cerr << "Error ending connection with the Virtual Robot (HTTP response code " << endConnResponse.code << ")" << std::endl;
		return 1;
	}
	if (endConnResponse.message != "session_invalidated") {
		std::cerr << "Error ending connection with the Virtual Robot. Response message received: " << endConnResponse.message << std::endl;
		return 1;
	}

	delete (HttpCookieSession*)vrSession;
	inited = false;
	return 0;
}

// Asks the Service Manager for the AIO
int FionaClient::requestAio() {
	std::stringstream smUrlStream;
	smUrlStream << protocol << "//" << smUrl << "/ServiceMngHandler/ServiceMngHandlerServlet";
	HttpCookieSession smSession (smUrlStream.str());
	std::vector<HttpParam> aioReqparams;
	aioReqparams.push_back (HttpParam{"action","init"});
	aioReqparams.push_back (HttpParam{"user",usermail});
	aioReqparams.push_back (HttpParam{"avname",avname});
	HttpResponse aioReqResponse = smSession.performHttpGet (aioReqparams);
	if (aioReqResponse.code != HTTP_SUCCESS) {
		std::cerr << "Error obtaining AIO from Service Manager (HTTP response code " << aioReqResponse.code << ")" << std::endl;
		return 1;
	}
	if (!aioReqResponse.message.empty())
		aioUrl = aioReqResponse.message;
	else {
		std::cerr << "No data received from the Service Manager. Assuming default AIO" << std::endl;
		aioUrl = defAioUrl;
	}

	return 0;
}

// Initializes the Virtual Robot
int FionaClient::initializeVr () {
	std::stringstream vrUrlStream;
	vrUrlStream << protocol << "//" << aioUrl << "/FionaHandler/FionaEmbedServlet";
	vrSession = new HttpCookieSession (vrUrlStream.str());
	roomId = generateRandomRoomId(32);
	std::cerr << "Randomly generated room Id for the RTMP server: " << roomId << std::endl;
	std::vector<HttpParam> vrInitparams;
	vrInitparams.push_back (HttpParam{"action","init"});
	vrInitparams.push_back (HttpParam{"user",usermail});
	vrInitparams.push_back (HttpParam{"room",roomId});
	vrInitparams.push_back (HttpParam{"av",avatarId});
	std::cerr << "Initializing connection with the Virtual Robot..." << std::endl;
	HttpResponse vrInitResponse = ((HttpCookieSession*)vrSession)->performHttpGet (vrInitparams);
	if (vrInitResponse.code != HTTP_SUCCESS) {
		std::cerr << "Error initializing Virtual Robot (HTTP response code " << vrInitResponse.code << ")" << std::endl;
		return 1;
	}
	if (vrInitResponse.cookies.empty()) {
		std::cerr << "No session cookie received. Cannot start session with the AIO" << std::endl;
		return 1;
	}
	if (vrInitResponse.message.empty() || std::find_if(vrInitResponse.message.begin(),
	  vrInitResponse.message.end(), [](char c) {return !(std::isdigit(c) || c == ' ');})
	  != vrInitResponse.message.end()) { // check that the response is only formed of numbers and spaces
		std::cerr << "Error initializing Virtual Robot. Response message received: " << vrInitResponse.message << std::endl;
		return 1;
	}
	for (std::list<std::string>::iterator it = vrInitResponse.cookies.begin(); it != vrInitResponse.cookies.end(); it++)
		std::cerr << "Session cookie: " << *it << std::endl;

	return 0;
}

//Sets the cookies (they are actually set, probably unnecesary)
int FionaClient::setCookies () {
	std::vector<HttpParam> setCookparams;
	setCookparams.push_back (HttpParam{"action","setcookies"});
	setCookparams.push_back (HttpParam{"user",usermail});
	setCookparams.push_back (HttpParam{"av",avatarId});
	HttpResponse setCookResponse = ((HttpCookieSession*)vrSession)->performHttpGet (setCookparams);
	if (setCookResponse.code != HTTP_SUCCESS) {
		std::cerr << "Error setting cookies (HTTP response code " << setCookResponse.code << ")" << std::endl;
		return 1;
	}
	if (!setCookResponse.message.empty()) {
		std::cerr << "Error setting cookies. Response message received: " << setCookResponse.message << std::endl;
		return 1;
	}

	return 0;
}

// Sends a sigCheck once to the Virtual Robot (it should be called every 5 seconds)
int FionaClient::sendSigCheck () {
	std::vector<HttpParam> sigCheckparams;
	sigCheckparams.push_back (HttpParam{"action","sigcheck"});
	sigCheckparams.push_back (HttpParam{"user",usermail});
	sigCheckparams.push_back (HttpParam{"av",avatarId});
	HttpResponse sigCheckResponse = ((HttpCookieSession*)vrSession)->performHttpGet (sigCheckparams);
	if (sigCheckResponse.code != HTTP_SUCCESS) {
		std::cerr << "Error sending sigCheck to Virtual Robot (HTTP response code " << sigCheckResponse.code << ")" << std::endl;
		return 1;
	}
	if (sigCheckResponse.message != "true") {
		std::cerr << "Error sending sigCheck to Virtual Robot. Response message received: " << sigCheckResponse.message << std::endl;
		return 1;
	}

	return 0;
}

// Initializes the thread that reads the default input audio device and writes it on the output stream
void FionaClient::initInAudioThread () {
	std::stringstream instreamNameStream;
	instreamNameStream << "rtmp://" << rtmpServerUrl << "/FionaRed5/" << roomId << "/usercam conn=S:" << usermail << " conn=S:" << usrid2 << " live=1";
	inAudioThread = new InAudioStream (instreamNameStream.str());
}

// Initializes the thread that reads the input audio stream and plays it on the default audio device
void FionaClient::initOutAudioThread () {
	std::stringstream outstreamNameStream;
	outstreamNameStream << "rtmp://" << rtmpServerUrl << "/FionaRed5/" << roomId << "/avatar conn=S:" << usermail << " conn=S:" << usrid2 << " live=1";
	outAudioThread = new OutAudioStream (outstreamNameStream.str());
}

// Generates a random string of the length indicated as parameter using the characters from the dictionary
std::string FionaClient::generateRandomRoomId (int length){
const std::string dictionary = "abcdefghijklmnopqrstuvwxyz0123456789";
std::stringstream randomRoom;
std::mt19937 rg(std::random_device{}());
std::uniform_int_distribution<> dist(0, dictionary.size()-1);
for (int i = 0; i < length; i++)
	randomRoom << dictionary [dist (rg)];
return randomRoom.str();
}
