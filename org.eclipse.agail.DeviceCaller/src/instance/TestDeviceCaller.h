#ifndef __AGAIL_TESTDEVICECALLER_H__
#define __AGAIL_TESTDEVICECALLER_H__

#include "../base/DeviceCaller.h"

#define AGAIL_TESTDEVICE_METHOD_PROPRIETARY ProprietaryMethod

namespace AGAIL {
    class TestDeviceCaller;
}

class AGAIL::TestDeviceCaller : public DeviceCaller {
    public:
	TestDeviceCaller(std::string address);

	virtual void AGAIL_TESTDEVICE_METHOD_PROPRIETARY (); // example proprietary method

    protected:
	static const std::string TEST_DEVICE;
};

#endif
