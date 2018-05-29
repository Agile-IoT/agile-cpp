#ifndef __AGAIL_RECORDOBJECT_H__
#define __AGAIL_RECORDOBJECT_H__

namespace AGAIL {
    class RecordObject;
}

class AGAIL::RecordObject {
    public:
	RecordObject (std::string deviceID, std::string componentID, std::string value, std::string unit, std::string format, long lastUpdate) : deviceID(deviceID), componentID(componentID), value(value), unit(unit), format(format), lastUpdate(lastUpdate) {}

	const std::string deviceID;
	const std::string componentID;
	const std::string value;
	const std::string unit;
	const std::string format;
	const long lastUpdate;
};

#endif
