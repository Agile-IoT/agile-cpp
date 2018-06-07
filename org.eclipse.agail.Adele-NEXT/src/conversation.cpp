#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>

#include "fionaclient.h"

const std::string usermail = "next-test@yopmail.com";
const std::string usrid2 = "335753f7ce12c345b45975b259fa3d9d";
const std::string avname = "Next";
const std::string avatarId = "864";

const Protocol protocol = http;
const std::string smUrl = "sm.adelerobots.com";
const std::string defAioUrl = "aio0001.adelerobots.com";
const std::string rtmpServerUrl = "195.55.126.59";

bool finnish = false;

void sig_handler (int sig) {
	finnish = true;
}

int main () {
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = sig_handler;
   	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	FionaClient myClient;

	// Set the avatar identification parameters
	if (myClient.setAvatar (usermail, usrid2, avatarId, avname))
		return 1;

	// Set the protocol
	if( myClient.setProtocol (protocol))
		return 1;

	// Set the Service Manager
	if (myClient.setSm (smUrl))
		return 1;

	// Set the default AIO (in case the Service Manager does not respond)
	if (myClient.setDefAio (defAioUrl))
		return 1;

	// Set the RTMP server
	if (myClient.setRtmpServer (rtmpServerUrl))
		return 1;

	// Initialize the connection with the Virtual Robot
	if (myClient.initConnection())
		return 1;

	// Wait for some time
	//for (int i = 30; i > 0; i--) {
	//	std::this_thread::sleep_for (std::chrono::seconds(1));
	//	std::cout << i << std::endl;
	//}

	while (!finnish) {
		std::this_thread::sleep_for (std::chrono::seconds(1));
	}

	// End the connection with the Virtual Robot
	if (myClient.endConnection())
		return 1;

	return 0;
}
