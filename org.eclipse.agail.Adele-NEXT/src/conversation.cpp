#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <signal.h>

#include "fionaclient.h"
#include "Storage.h"

bool finnish = false;

void sig_handler (int sig) {
	finnish = true;
}

int getFionaClientParams (std::string& usermail, std::string& usrid2, std::string& avname, std::string& avatarId, std::string& protocol, std::string& smUrl, std::string& defAioUrl, std::string& rtmpServerUrl);

int main () {
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = sig_handler;
   	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	FionaClient myClient;

	// Read the parameters from the database
	std::string usermail;
	std::string usrid2;
	std::string avname;
	std::string avatarId;
	std::string protocol;
	std::string smUrl;
	std::string defAioUrl;
	std::string rtmpServerUrl;
	if (getFionaClientParams (usermail, usrid2, avname, avatarId, protocol, smUrl, defAioUrl, rtmpServerUrl))
		return 1;

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

	// Wait until a SIGINT signal is received
	while (!finnish) {
		std::this_thread::sleep_for (std::chrono::seconds(1));
	}

	// End the connection with the Virtual Robot
	if (myClient.endConnection())
		return 1;

	return 0;
}

int getFionaClientParams (std::string& usermail, std::string& usrid2, std::string& avname, std::string& avatarId, std::string& protocol, std::string& smUrl, std::string& defAioUrl, std::string& rtmpServerUrl) {
    // Open and parse the configuration file
    Json::Value conf;
    std::ifstream config_doc ("org.eclipse.agail.Adele-NEXT/conf/idm-entity-storage-conf.json", std::ifstream::binary);
    if (!config_doc.is_open()) {
        std::cout << "Error opening the conf file" << std::endl;
        return 1;
    }
    Json::Reader jsonReader;
    bool jsonError = jsonReader.parse (config_doc, conf);
    if (!jsonError) {
        std::cout << "Error reading the conf file: " << jsonReader.getFormattedErrorMessages() << std::endl;
        return 1;
    }

    // Read the FIONA client entry
    AGAIL::Storage storage (conf);
    std::string entity_id = "fiona";
    std::string entity_type = "/client";
    Json::Value data;
    int ret = storage.readEntity (entity_id, entity_type, data);
    if (ret != 0) {
	storage.disconnect ();
	std::cout << "Error reading FIONA client entry from the database" << std::endl;
	return 1;
    }
    usermail = data["usermail"].asString();
    usrid2 = data["usrid2"].asString();
    avname = data["avname"].asString();
    avatarId = data["avatarId"].asString();
    protocol = data["protocol"].asString();
    smUrl = data["smUrl"].asString();
    defAioUrl = data["defAioUrl"].asString();
    rtmpServerUrl = data["rtmpServerUrl"].asString();

    // Close database
    storage.disconnect ();
}
