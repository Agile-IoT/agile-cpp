#ifndef __AGAIL_SPEAKERCALLER_H__
#define __AGAIL_SPEAKERCALLER_H__

#include "../base/DeviceCaller.h"

extern "C"{
#include <soundio/soundio.h>
}

namespace AGAIL {
    class SpeakerCaller;
}

class AGAIL::SpeakerCaller : public DeviceCaller {
    public:
	SpeakerCaller(std::string address);

    protected:
	static const std::string SPEAKER_DEVICE;
};

#endif
