#ifndef __NEXT_INAUDIOSTREAM_H__
#define __NEXT_INAUDIOSTREAM_H__

#include <thread>

#include "MicrophoneCaller.h"

extern "C"{
#include <soundio/soundio.h>
#include <libavformat/avformat.h>
}

// A wrapper around a single output AVStream
struct OutputStream {
    AVStream *st;
    int64_t next_pts;	// pts of the next frame that will be generated
    int samples_count;
    AVFrame *out_frame;
    AVFrame *in_frame;
    struct SwrContext *swr_ctx;
};

// Opens a separate thread to record the microphone audio input to the file or stream indicated as parameter
class InAudioStream{
public:
	InAudioStream(std::string outstream_name);	// constructor
	void stop();	// ends the thread
private:
	std::string outstream_name;
	AVFormatContext *out_format_ctx = NULL;
	AVOutputFormat *out_format;
	OutputStream out_audio_stream = { 0 };
	bool isRtmp = false;	// patch to get an appropriate format and sample rate for RTMP output streams
	AGAIL::MicrophoneCaller* myMicrophone;
	SoundIoFormat sample_format;
	int sample_rate;
	int n_channels;
	std::string buffer;
	std::thread th;
	bool running = true;
	void performThread();
	int initInDevice();
	int initOutStream();
	int run();
	int finish();
	AVSampleFormat get_sample_format();
	int add_stream(AVCodec **audio_codec, enum AVCodecID audio_codec_id);
	int open_audio(AVCodec *audio_codec);
	AVFrame *alloc_audio_frame(enum AVSampleFormat out_sample_format, uint64_t out_channel_layout, int out_sample_rate, int out_nb_samples);
	AVFrame *get_audio_frame();
	int write_frame(const AVRational *time_base, AVPacket *out_pkt);
	void log_packet(const AVPacket *out_pkt);
};

#endif
