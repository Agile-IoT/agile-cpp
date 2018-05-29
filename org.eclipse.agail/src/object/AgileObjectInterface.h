#ifndef __AGAIL_AGILEOBJECTINTERFACE_H__
#define __AGAIL_AGILEOBJECTINTERFACE_H__

#include <string>

namespace AGAIL {
    class AgileObjectInterface;
}

class AGAIL::AgileObjectInterface {
    public:
	virtual void dbusConnect(const std::string busName, const std::string busPath, const std::string busIntrospection) = 0;
	virtual void dbusDisconnect() = 0;
};

#endif
