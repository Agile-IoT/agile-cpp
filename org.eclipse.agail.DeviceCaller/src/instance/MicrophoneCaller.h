#ifndef __AGAIL_MICROPHONECALLER_H__
#define __AGAIL_MICROPHONECALLER_H__

#include "../base/DeviceCaller.h"

extern "C"{
#include <soundio/soundio.h>
}

namespace AGAIL {
    class MicrophoneCaller;
}

class AGAIL::MicrophoneCaller : public DeviceCaller {
    public:
	MicrophoneCaller(std::string address);

    protected:
	static const std::string MICROPHONE_DEVICE;
};

#endif
