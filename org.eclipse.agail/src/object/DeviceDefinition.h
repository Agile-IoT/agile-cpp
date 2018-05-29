#ifndef __AGAIL_DEVICEDEFINITION_H__
#define __AGAIL_DEVICEDEFINITION_H__

#include "DeviceComponent.h"

namespace AGAIL {
    class DeviceDefinition;
}

class AGAIL::DeviceDefinition {
    public:
	DeviceDefinition (std::string deviceID, std::string address, std::string name, std::string description, std::string protocol, std::string path, std::list<AGAIL::DeviceComponent> profile) : deviceID(deviceID), address(address), name(name), description(description), protocol(protocol), path(path), profile(profile) {};

	const std::string deviceID;
	const std::string address;
	const std::string name;
	const std::string description;
	const std::string protocol;
	const std::string path;
	const std::list<AGAIL::DeviceComponent> profile;
};

#endif
