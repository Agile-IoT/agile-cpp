#ifndef __AGAIL_SPEAKER_H__
#define __AGAIL_SPEAKER_H__

#include "../base/DeviceImp.h"

extern "C"{
#include <soundio/soundio.h>
}

namespace AGAIL {
    class Speaker;
}

class AGAIL::Speaker : public AGAIL::DeviceImp {
    public:
	Speaker (std::string deviceName, std::string address); // constructor
	Speaker (AGAIL::DeviceDefinition definition); // constructor

    protected:
	static const std::string SPEAKER_DEVICE;
	static const std::string SPEAKER_PROTOCOL;

	bool isConnected = false;

	// AGILE::Device interface methods
	virtual void AGAIL_DEVICE_METHOD_CONNECT () override; // sets up and initializes a connection to the device
        virtual void AGAIL_DEVICE_METHOD_DISCONNECT () override; // disconnects from the device
        virtual void AGAIL_DEVICE_METHOD_WRITE (std::string componentID, std::string payload) override; // writes data to the component

	// Virtual functions called from g_bus_own_name() function
	virtual void onBusAcquiredCb(GDBusConnection *, const gchar *) override;
        virtual void onNameAcquiredCb(GDBusConnection *, const gchar *) override;
        virtual void onNameLostCb(GDBusConnection *, const gchar *) override;

	// libsoundio variables and methods
        SoundIo *soundio;
        SoundIoDevice *device;
        SoundIoOutStream *outstream;
        SoundIoRingBuffer *ring_buffer;
	static void write_callback (struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max); // callback function to send the frames to the speaker

	// variables to be set before connecting to the device
	SoundIoFormat sample_format = SoundIoFormatInvalid;
	int sample_rate = 0;
	int n_channels = 0;

    private:
	void writeAudio (std::string payload);
};

#endif
