#ifndef __NEXT_OUTAUDIOSTREAM_H__
#define __NEXT_OUTAUDIOSTREAM_H__

#include <thread>

#include "SpeakerCaller.h"

extern "C"{
#include <libavformat/avformat.h>
#include <soundio/soundio.h>
}

// Opens a separate thread to play the audio file or stream indicated as parameter
class OutAudioStream{
public:
	OutAudioStream(std::string instream_name);	// constructor
	void stop();	// ends the thread
private:
	std::string instream_name;
	AVFormatContext *in_format_ctx = NULL;
	AVCodecContext *audio_decod_ctx;
	int in_audio_stream_idx = -1;
	AVPacket in_pkt;
	AVFrame *in_frame;
	AGAIL::SpeakerCaller* mySpeaker;
	std::thread th;
	bool running = true;
	void performThread();
	int initInStream();
	int initOutDevice();
	int run();
	void finish();
	int open_decod_context();
	SoundIoFormat get_sample_format();
};

#endif
