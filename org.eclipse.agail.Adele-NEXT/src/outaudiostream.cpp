#include "outaudiostream.h"

#include <iostream> // TODO: substitute by log library

#include <chrono>

// To avoid av_err2str macro errors in C++ (it only works in C)
#undef av_err2str
#define av_err2str(errnum) \
av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE),AV_ERROR_MAX_STRING_SIZE, errnum)

// Constructor: initializes the thread that plays the audio stream
OutAudioStream::OutAudioStream(std::string instream_name){
	this->instream_name = instream_name;
	th = std::thread(&OutAudioStream::performThread, this);
}

// Ends the thread that plays the audio stream
void OutAudioStream::stop(){
	running = false;
	th.join();
}

// Function implemented in the thread: plays the audio stream
void OutAudioStream::performThread(){
	if(initInStream())
		return;
	if (initOutDevice())
		return;
	if (run())
		return;
	finish();
}

// Initializes the input audio stream (uses the ffmpeg library)
int OutAudioStream::initInStream(){
	int ret;

	// Register all formats and codecs
	av_register_all();

	// Initialize network components
	if((ret = avformat_network_init()) < 0){
		fprintf(stderr, "Could not initialize network components to open the input audio stream (%s)\n", av_err2str(ret));
		return 1;
	}

	// Open input stream, and allocate format context
	if ((ret = avformat_open_input(&in_format_ctx, instream_name.c_str(), NULL, NULL)) < 0) {	// NOTE: with RTMP it returns a server error
		fprintf(stderr, "Could not open input stream %s (%s)\n", instream_name.c_str(), av_err2str(ret));
		return 1;
	}

	// Retrieve stream information
	in_format_ctx->probesize = 131072;	// this way, we spend less time in avformat_find_stream_info
	if ((ret = avformat_find_stream_info(in_format_ctx, NULL)) < 0) {
		fprintf(stderr, "Could not find stream information (%s)\n", av_err2str(ret));
		return 1;
	}

	// Initialize audio decoder
	if ((ret = open_decod_context())) {
		fprintf(stderr, "Could not find audio stream in the input, aborting (%s)\n", av_err2str(ret));
		return 1;
	}
	AVStream *in_audio_stream = in_format_ctx->streams[in_audio_stream_idx];
	audio_decod_ctx = in_audio_stream->codec;

	// Dump input information to stderr
	av_dump_format(in_format_ctx, 0, instream_name.c_str(), 0);

	// Initialize packet
	av_init_packet(&in_pkt);
	in_pkt.data = NULL;
	in_pkt.size = 0;

	// Allocate input frame
	in_frame = av_frame_alloc();
	if (!in_frame) {
		fprintf(stderr, "Could not allocate frame (%s)\n", av_err2str(AVERROR(ENOMEM)));
		return 1;
	}

	return 0;
}

// Initializes the output device
int OutAudioStream::initOutDevice(){
	// Create speaker object
	mySpeaker = new AGAIL::SpeakerCaller("0000");
	if(!mySpeaker->isConnected()){
		fprintf(stderr, "Could not connect to D-Bus\n");
		return 1;
	}

	// Set parameters and connect to speaker
	SoundIoFormat sample_format = get_sample_format();
	if(sample_format == SoundIoFormatInvalid)
                return 1;
	mySpeaker->AGAIL_DEVICE_METHOD_WRITE("sample_format", std::to_string((int)sample_format));
	mySpeaker->AGAIL_DEVICE_METHOD_WRITE("sample_rate", std::to_string(audio_decod_ctx->sample_rate));
	mySpeaker->AGAIL_DEVICE_METHOD_WRITE("n_channels", std::to_string(audio_decod_ctx->channels));
	mySpeaker->AGAIL_DEVICE_METHOD_CONNECT();
	fprintf(stderr, "Connected to speaker\n");
	fprintf(stderr, "Output audio device stream: %s %dHz %s interleaved\n", soundio_channel_layout_get_default(audio_decod_ctx->channels)->name, audio_decod_ctx->sample_rate, soundio_format_string(sample_format));

	return 0;
}

// Reads frames from the input stream and sends them to the output device
int OutAudioStream::run(){
	int ret;
	fprintf(stderr, "Demuxing audio from input stream to default output audio device...\n");

	// Iterate until stop() function is called (or an error occurs)
	while(running){
		// Read a new audio packet (and discard video packets)
		do{
			if((ret = av_read_frame(in_format_ctx, &in_pkt)) < 0)
			{
				fprintf(stderr, "Input audio stream finished or error reading input audio stream (%s)\n", av_err2str(ret));
				return 1;
			}
			if(in_pkt.stream_index != in_audio_stream_idx)
				av_packet_unref(&in_pkt);
		}while(in_pkt.stream_index != in_audio_stream_idx);
		AVPacket orig_pkt = in_pkt; // we keep a copy of the read package to free it later

		// Read the content of the packet (in order to do so, access it as many times as necessary)
		do {
			int got_frame = 0;
			// Decode audio frame
			if ((ret = avcodec_decode_audio4(audio_decod_ctx, in_frame, &got_frame, &in_pkt)) < 0) {
				fprintf(stderr, "Error decoding audio frame (%s)\n", av_err2str(ret));
				return 1;
			}
			// Some audio decoders decode only part of the packet, and have to be
			// called again with the remainder of the packet data.
			// Also, some decoders might over-read the packet.
			int decoded = FFMIN(ret, in_pkt.size);

			// Write the read frames to the AGILE device
			if (got_frame) {
				size_t unpadded_linesize = in_frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)in_frame->format);
				// Write the raw audio data samples of the first plane. This works
				// fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
				// most audio decoders output planar audio, which uses a separate
				// plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
				// In other words, this code will write only the first audio channel
				// in these cases.
				// You should use libswresample or libavfilter to convert the frame
				// to packed data.
				mySpeaker->AGAIL_DEVICE_METHOD_WRITE ("audio", std::string ((char*)in_frame->extended_data[0], unpadded_linesize));
			}

			// Update the packet to read the next data
			in_pkt.data += decoded;
			in_pkt.size -= decoded;
		} while (in_pkt.size > 0 && running);

		// Free the original packet
		av_packet_unref(&orig_pkt);
	}

	return 0;
}

// Frees the memory
void OutAudioStream::finish(){
	fprintf(stderr, "Finishing audio output...\n");

	mySpeaker->AGAIL_DEVICE_METHOD_DISCONNECT();
	delete mySpeaker;

	avcodec_close(audio_decod_ctx);
	avformat_close_input(&in_format_ctx);
	av_frame_free(&in_frame);
	avformat_network_deinit();
}

// Initializes the audio decoder
int OutAudioStream::open_decod_context()
{
    int ret;
    AVMediaType type = AVMEDIA_TYPE_AUDIO;

    // Find audio stream
    if ((ret = av_find_best_stream(in_format_ctx, type, -1, -1, NULL, 0)) < 0){
        fprintf(stderr, "Could not find %s stream in input stream (%s)\n",
                av_get_media_type_string(type), av_err2str(ret));
        return 1;
    }
    in_audio_stream_idx = ret;
	AVStream *st = in_format_ctx->streams[in_audio_stream_idx];

	// Find decoder for the stream
	AVCodecContext *decod_ctx = st->codec;
	AVCodec *decod = avcodec_find_decoder(decod_ctx->codec_id);
	if (!decod) {
		fprintf(stderr, "Failed to find %s decoder (%s)\n",
				av_get_media_type_string(type), av_err2str(AVERROR(EINVAL)));
		return 1;
	}

	// Initialize decoder
	if ((ret = avcodec_open2(decod_ctx, decod, NULL)) < 0){
		fprintf(stderr, "Failed to initialize %s decoder (%s)\n",
				av_get_media_type_string(type), av_err2str(ret));
		return 1;
	}

    return 0;
}

// Sets the output stream sample format according to the input stream sample format
SoundIoFormat OutAudioStream::get_sample_format(){
	SoundIoFormat sample_format;

	switch(audio_decod_ctx->sample_fmt){
	case AV_SAMPLE_FMT_U8:
	case AV_SAMPLE_FMT_U8P:
		sample_format = SoundIoFormatU8;
		break;
	case AV_SAMPLE_FMT_S16:
	case AV_SAMPLE_FMT_S16P:
		sample_format = SoundIoFormatS16NE;
		break;
	case AV_SAMPLE_FMT_S32:
	case AV_SAMPLE_FMT_S32P:
		sample_format = SoundIoFormatS32NE;
		break;
	case AV_SAMPLE_FMT_FLT:
	case AV_SAMPLE_FMT_FLTP:
		sample_format = SoundIoFormatFloat32NE;
		break;
	case AV_SAMPLE_FMT_DBL:
	case AV_SAMPLE_FMT_DBLP:
		sample_format = SoundIoFormatFloat64NE;
		break;
	default:
		fprintf(stderr, "Input sample format not supported.\n");
		return SoundIoFormatInvalid;
	}
	if (audio_decod_ctx->channels > 1){
		switch(audio_decod_ctx->sample_fmt){
		case AV_SAMPLE_FMT_U8P:
		case AV_SAMPLE_FMT_S16P:
		case AV_SAMPLE_FMT_S32P:
		case AV_SAMPLE_FMT_FLTP:
		case AV_SAMPLE_FMT_DBLP:
			// Designed only for interleaved samples (or for mono). For supporting planar sample formats
			// libswresample should be used in the main loop
			fprintf(stderr, "Warning: planar sample format not supported yet. Taking only first audio channel.\n");
		default:
			break;
		}
	}
	return sample_format;
}
