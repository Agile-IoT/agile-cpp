#ifndef __AGAIL_MICROPHONE_H__
#define __AGAIL_MICROPHONE_H__

#include "../base/DeviceImp.h"

extern "C"{
#include <soundio/soundio.h>
}

namespace AGAIL {
    class Microphone;
}

class AGAIL::Microphone : public AGAIL::DeviceImp {
    public:
	Microphone (std::string deviceName, std::string address); // constructor
	Microphone (AGAIL::DeviceDefinition definition); // constructor

    protected:
	static const std::string MICROPHONE_DEVICE;
	static const std::string MICROPHONE_PROTOCOL;

	bool isConnected = false;

	// AGILE::Device interface methods
	virtual void AGAIL_DEVICE_METHOD_CONNECT () override; // sets up and initializes a connection to the device
        virtual void AGAIL_DEVICE_METHOD_DISCONNECT () override; // disconnects from the device
        virtual AGAIL::RecordObject AGAIL_DEVICE_METHOD_READ (std::string componentID) override; // reads data from the component

	// Virtual functions called from g_bus_own_name() function
	virtual void onBusAcquiredCb(GDBusConnection *, const gchar *) override;
        virtual void onNameAcquiredCb(GDBusConnection *, const gchar *) override;
        virtual void onNameLostCb(GDBusConnection *, const gchar *) override;

	// libsoundio variables and methods
        SoundIo *soundio;
        SoundIoDevice *device;
        SoundIoInStream *instream;
        SoundIoRingBuffer *ring_buffer;
        static void read_callback (struct SoundIoInStream *instream, int frame_count_min, int frame_count_max); // callback function to get the frames from the microphone

	// variables set when connecting to the device
        SoundIoFormat sample_format = SoundIoFormatInvalid;
        int sample_rate = 0;
        int n_channels = 0;

    private:
	AGAIL::RecordObject readAudio (std::string componentID);
	AGAIL::RecordObject generateReadRecord (std::string componentID, std::string value);
};

#endif
