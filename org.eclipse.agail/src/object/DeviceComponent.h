#ifndef __AGAIL_DEVICECOMPONENT_H__
#define __AGAIL_DEVICECOMPONENT_H__

namespace AGAIL {
    class DeviceComponent;
}

class AGAIL::DeviceComponent {
    public:
	DeviceComponent (std::string componentID, std::string unit) : componentID(componentID), unit(unit) {}

	const std::string componentID;
	const std::string unit;
};

#endif
