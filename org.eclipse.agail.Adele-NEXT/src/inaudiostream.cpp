#include "inaudiostream.h"

#include <iostream> // TODO: substitute by log library

#include <chrono>

extern "C"{
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libswresample/swresample.h>
#include <libavutil/timestamp.h>
}

// To avoid av_err2str macro errors in C++ (it only works in C)
#undef av_err2str
#define av_err2str(errnum) \
av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE),AV_ERROR_MAX_STRING_SIZE, errnum)
#undef av_ts2str
#define av_ts2str(ts) \
av_ts_make_string((char*)__builtin_alloca(AV_TS_MAX_STRING_SIZE), ts)
#undef av_ts2timestr
#define av_ts2timestr(ts, tb) \
av_ts_make_time_string((char*)__builtin_alloca(AV_TS_MAX_STRING_SIZE), ts, tb)

#define ONE_CHANNEL

// Constructor: initializes the thread that plays the audio stream
InAudioStream::InAudioStream(std::string outstream_name){
	this->outstream_name = outstream_name;
	isRtmp = !outstream_name.compare(0, strlen("rtmp:"), "rtmp:");
	th = std::thread(&InAudioStream::performThread, this);
}

// Ends the thread that plays the audio stream
void InAudioStream::stop(){
	running = false;
	th.join();
}

// Function implemented in the thread: plays the audio stream
void InAudioStream::performThread(){
	if (initInDevice())
		return;
	if(initOutStream())
		return;
	if (run())
		return;
	if (finish())
		return;
}

// Initializes the input device (uses the libsoundio library)
int InAudioStream::initInDevice(){
	// Create microphone object
	myMicrophone = new AGAIL::MicrophoneCaller("0000");
	if (!myMicrophone->isConnected()) {
		fprintf(stderr, "Could not connect to D-Bus\n");
		return 1;
	}

	// Connect to microphone and get parameters
	myMicrophone->AGAIL_DEVICE_METHOD_CONNECT();
	fprintf(stderr, "Connected to microphone\n");
	sample_format = (SoundIoFormat)std::stoi (myMicrophone->AGAIL_DEVICE_METHOD_READ ("sample_format").value);
	sample_rate = std::stoi (myMicrophone->AGAIL_DEVICE_METHOD_READ ("sample_rate").value);
	n_channels = std::stoi (myMicrophone->AGAIL_DEVICE_METHOD_READ ("n_channels").value);
	std::cout << "Input audio device stream: " << soundio_channel_layout_get_default(n_channels)->name << " " << sample_rate << "Hz " << soundio_format_string (sample_format) << " interleaved" << std::endl;

	return 0;
}

// Initializes the output audio stream (uses the ffmpeg library)
int InAudioStream::initOutStream(){
	int ret;

	// Register all formats and codecs
	av_register_all();

	// Initialize network components
	if((ret = avformat_network_init()) < 0){
		fprintf(stderr, "Could not initialize network components to open the input audio stream (%s)\n", av_err2str(ret));
		return 1;
	}

    // Allocate the output media context
	if (isRtmp)
		ret = avformat_alloc_output_context2(&out_format_ctx, NULL, "flv", outstream_name.c_str());	// sets output format to FLV
	else{
		ret = avformat_alloc_output_context2(&out_format_ctx, NULL, NULL, outstream_name.c_str());
		if (!out_format_ctx) {
			fprintf(stderr, "Could not deduce output format from file extension: using MPEG.\n");
			ret = avformat_alloc_output_context2(&out_format_ctx, NULL, "mpeg", outstream_name.c_str());
		}
	}
    if (!out_format_ctx){
    	fprintf(stderr, "Could not allocate output media context (%s)\n", av_err2str(ret));
        return 1;
    }
    out_format = out_format_ctx->oformat;

    // Add the audio stream using the default format encoder and initialize the encoder
    if (out_format->audio_codec == AV_CODEC_ID_NONE){
    	fprintf(stderr, "No default format encoder found for output audio stream\n");
		return 1;
    }
    AVCodec *audio_codec;
    if(add_stream(&audio_codec, out_format->audio_codec))
    	return 1;

    // Now that all the parameters are set, we can open the audio encoder and allocate the necessary encode buffers
    if(open_audio(audio_codec))
    	return 1;

    // Dump output information to stderr
    av_dump_format(out_format_ctx, 0, outstream_name.c_str(), 1);

    // Open the output audio stream, if needed
    if (!(out_format->flags & AVFMT_NOFILE)) {
        if ((ret = avio_open(&out_format_ctx->pb, outstream_name.c_str(), AVIO_FLAG_WRITE)) < 0) {
            fprintf(stderr, "Could not open output audio stream '%s' (%s)\n", outstream_name.c_str(), av_err2str(ret));
            return 1;
        }
    }

    // Write the stream header, if any
    if ((ret = avformat_write_header(out_format_ctx, NULL)) < 0) {
        fprintf(stderr, "Error occurred when writing header to file (%s)\n", av_err2str(ret));
        return 1;
    }

	return 0;
}

// Reads frames from the input device and sends them to the output stream
int InAudioStream::run(){
	int ret;
	fprintf(stderr, "Getting audio from default input audio device and muxing it to output stream...\n");

	// Iterate until stop() function is called (or an error occurs)
	AVCodecContext *audio_codec_ctx = out_audio_stream.st->codec;
	int got_packet;
	while (running) {
		// Initialize packet and frames
		AVPacket out_pkt = { 0 }; // data and size must be 0
		av_init_packet(&out_pkt);
		AVFrame *in_frame = get_audio_frame();
		AVFrame *out_frame = NULL;
		if (in_frame) {
			// Convert samples from native format to destination codec format, using the resampler
			// Compute destination number of samples
			int dst_nb_samples = av_rescale_rnd(swr_get_delay(out_audio_stream.swr_ctx, in_frame->sample_rate) + in_frame->nb_samples,
					audio_codec_ctx->sample_rate, in_frame->sample_rate, AV_ROUND_UP);
			// When we pass a frame to the encoder, it may keep a reference to it internally
			// Make sure we do not overwrite it here
			if ((ret = av_frame_make_writable(out_audio_stream.out_frame)) < 0){
				fprintf(stderr, "Could not ensure that the output frame is writable (%s)\n", av_err2str(ret));
				return 1;
			}
			// Convert frame to destination format
			if ((ret = swr_convert(out_audio_stream.swr_ctx,
					out_audio_stream.out_frame->data, dst_nb_samples,
							  (const uint8_t **)in_frame->data, in_frame->nb_samples)) < 0) {
				fprintf(stderr, "Error while converting input samples to destination format (%s)\n", av_err2str(ret));
				return 1;
			}
			// Update the output frame timestamps
			out_frame = out_audio_stream.out_frame;
			out_frame->pts = av_rescale_q(out_audio_stream.samples_count, (AVRational){1, audio_codec_ctx->sample_rate}, audio_codec_ctx->time_base);
			out_audio_stream.samples_count += dst_nb_samples;
		}
		// Encode audio frame (if no frame, flush the encoder)
		if ((ret = avcodec_encode_audio2(audio_codec_ctx, &out_pkt, out_frame, &got_packet)) < 0) {
			fprintf(stderr, "Error encoding audio frame (%s)\n", av_err2str(ret));
			return 1;
		}
		// Write packet to output audio stream
		if (got_packet) {
			if (write_frame(&audio_codec_ctx->time_base, &out_pkt))
				return 1;
		}
		// Check if there are still frames coming from the input audio device
		if(!(out_frame || got_packet))
		{
			fprintf(stderr, "No more frames coming from the input audio device\n");
			return 1;
		}
	}

	return 0;
}

// Frees the memory
int InAudioStream::finish(){
	int ret;
	fprintf(stderr, "Finishing audio input...\n");

	// Flush the encoder
	int got_packet;
	do {
		// encode empty audio frame
		AVPacket out_pkt = { 0 }; // data and size must be 0
		av_init_packet(&out_pkt);
		if ((ret = avcodec_encode_audio2(out_audio_stream.st->codec, &out_pkt, NULL, &got_packet)) < 0) {
			fprintf(stderr, "Error encoding audio frame (%s)\n", av_err2str(ret));
			return 1;
		}
		// write packet to output audio stream
		if (got_packet) {
			if (write_frame(&out_audio_stream.st->codec->time_base, &out_pkt))
				return 1;
		}
	} while (got_packet);

    // Write the trailer, if any. The trailer must be written before you
    // close the CodecContexts open when you wrote the header; otherwise
    // av_write_trailer() may try to use memory that was freed on
    // av_codec_close()
    av_write_trailer(out_format_ctx); // NOTE: with RTMP it returns errors about duration and filesize in header

    avcodec_close(out_audio_stream.st->codec);
	av_frame_free(&out_audio_stream.out_frame);
	av_frame_free(&out_audio_stream.in_frame);
	swr_free(&out_audio_stream.swr_ctx);
    if (!(out_format->flags & AVFMT_NOFILE))
        avio_closep(&out_format_ctx->pb);	// close the output audio stream, if needed
    avformat_free_context(out_format_ctx);
	avformat_network_deinit();

	myMicrophone->AGAIL_DEVICE_METHOD_DISCONNECT();
	delete myMicrophone;

	return 0;
}

// Adds an output stream
int InAudioStream::add_stream(AVCodec **audio_codec, enum AVCodecID audio_codec_id){
    // Find the encoder
    *audio_codec = avcodec_find_encoder(audio_codec_id);
    if (!(*audio_codec)) {
        fprintf(stderr, "Could not find audio encoder for '%s'\n", avcodec_get_name(audio_codec_id));
        return 1;
    }

    // Allocate output audio stream
    out_audio_stream.st = avformat_new_stream(out_format_ctx, *audio_codec);
    if (!out_audio_stream.st) {
        fprintf(stderr, "Could not allocate output audio stream\n");
        return 1;
    }

    // Set the best parameters for the output audio stream
    out_audio_stream.st->id = out_format_ctx->nb_streams-1;	// ? why?
    AVCodecContext *audio_codec_ctx = out_audio_stream.st->codec;
    // if possible, set the sample format to the same as the input stream
	if(get_sample_format() == AV_SAMPLE_FMT_NONE)
		return 1;
	audio_codec_ctx->sample_fmt = get_sample_format();
	if ((*audio_codec)->sample_fmts) {
		audio_codec_ctx->sample_fmt = (*audio_codec)->sample_fmts[0];
		for(int i = 0; (*audio_codec)->sample_fmts[i] != -1; i++)
			if ((*audio_codec)->sample_fmts[i] == get_sample_format())
				audio_codec_ctx->sample_fmt = get_sample_format();
	}
	// the bit rate is only used in non-constant quantizer encoding (set it just in case)
	audio_codec_ctx->bit_rate = 64000;
	// if possible, set the sample rate to the same as the input stream
	audio_codec_ctx->sample_rate = sample_rate;
	if ((*audio_codec)->supported_samplerates) {
		audio_codec_ctx->sample_rate = (*audio_codec)->supported_samplerates[0];
		for (int i = 0; (*audio_codec)->supported_samplerates[i]; i++)
			if ((*audio_codec)->supported_samplerates[i] == sample_rate)
				audio_codec_ctx->sample_rate = sample_rate;
	}
	if(isRtmp) // FLV only supports 44100, 22050 and 11025 bitrates
		audio_codec_ctx->sample_rate = (audio_codec_ctx->sample_rate/11025)*11025;
	// if possible, set the channel layout to the same as the input stream
	audio_codec_ctx->channel_layout = av_get_default_channel_layout(n_channels);
	if ((*audio_codec)->channel_layouts) {
		audio_codec_ctx->channel_layout = (*audio_codec)->channel_layouts[0];
		for (int i = 0; (*audio_codec)->channel_layouts[i]; i++)
			if ((*audio_codec)->channel_layouts[i] == (uint64_t)av_get_default_channel_layout(n_channels))
				audio_codec_ctx->channel_layout = av_get_default_channel_layout(n_channels);
	}
#ifdef ONE_CHANNEL
	audio_codec_ctx->channel_layout = av_get_default_channel_layout(1);
#endif
	audio_codec_ctx->channels = av_get_channel_layout_nb_channels(audio_codec_ctx->channel_layout);
	// set the time base according to the sample rate
	out_audio_stream.st->time_base = (AVRational){ 1, audio_codec_ctx->sample_rate };

    // Some formats want stream headers to be separate
    if (out_format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
    	audio_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    return 0;
}

// Opens the audio encoder, allocates the necessary encode buffers and initializes the resampling context
int InAudioStream::open_audio(AVCodec *audio_codec)
{
	int ret, nb_samples;
    AVCodecContext *audio_codec_ctx = out_audio_stream.st->codec;

    // Open encoder
    if ((ret = avcodec_open2(audio_codec_ctx, audio_codec, NULL)) < 0) {
        fprintf(stderr, "Could not open audio encoder: %s\n", av_err2str(ret));
        return 1;
    }

    // Set the number of samples per channel and frame
    if (audio_codec_ctx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = audio_codec_ctx->frame_size;

    // Allocate input and output audio frames
    out_audio_stream.out_frame = alloc_audio_frame(audio_codec_ctx->sample_fmt, audio_codec_ctx->channel_layout,
    		audio_codec_ctx->sample_rate, nb_samples);
    if (!out_audio_stream.out_frame)
    	return 1;
    out_audio_stream.in_frame = alloc_audio_frame(get_sample_format(), av_get_default_channel_layout(n_channels),
    		sample_rate, av_rescale_rnd(nb_samples, sample_rate, audio_codec_ctx->sample_rate, AV_ROUND_UP));
    if (!out_audio_stream.in_frame)
		return 1;

    // Create resampler context
    out_audio_stream.swr_ctx = swr_alloc();
	if (!out_audio_stream.swr_ctx) {
		fprintf(stderr, "Could not allocate resampler context\n");
		return 1;
	}
	// set resampling options
	av_opt_set_int       (out_audio_stream.swr_ctx, "in_channel_count",   av_get_channel_layout_nb_channels(out_audio_stream.in_frame->channel_layout), 0);
	av_opt_set_int       (out_audio_stream.swr_ctx, "in_sample_rate",     out_audio_stream.in_frame->sample_rate, 0);
	av_opt_set_sample_fmt(out_audio_stream.swr_ctx, "in_sample_fmt",      (AVSampleFormat)out_audio_stream.in_frame->format, 0);
	av_opt_set_int       (out_audio_stream.swr_ctx, "out_channel_count",  audio_codec_ctx->channels,       0);
	av_opt_set_int       (out_audio_stream.swr_ctx, "out_sample_rate",    audio_codec_ctx->sample_rate,    0);
	av_opt_set_sample_fmt(out_audio_stream.swr_ctx, "out_sample_fmt",     audio_codec_ctx->sample_fmt,     0);
	// initialize the resampling context
	if ((ret = swr_init(out_audio_stream.swr_ctx)) < 0) {
		fprintf(stderr, "Failed to initialize the resampling context (%s)\n", av_err2str(ret));
		return 1;
	}

	return 0;
}

// Allocates and initializes an audio frame
AVFrame *InAudioStream::alloc_audio_frame(enum AVSampleFormat out_sample_format,
                                  uint64_t out_channel_layout,
                                  int out_sample_rate, int out_nb_samples)
{
	int ret;

	// Allocate frame
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        return NULL;
    }

    // Set audio frame parameters
    frame->format = out_sample_format;
    frame->channel_layout = out_channel_layout;
    frame->sample_rate = out_sample_rate;
    frame->nb_samples = out_nb_samples;

    // Allocate audio frame buffer
    if (out_nb_samples) {
        if ((ret = av_frame_get_buffer(frame, 0)) < 0) {
            fprintf(stderr, "Error allocating an audio buffer (%s)\n", av_err2str(ret));
            return NULL;
        }
    }

    return frame;
}

// Gets a complete audio frame from the AGILE device
AVFrame *InAudioStream::get_audio_frame()
{
    AVFrame *frame = out_audio_stream.in_frame;

    // Copy the data from the AGILE device to the frame
    AGAIL::RecordObject input = myMicrophone->AGAIL_DEVICE_METHOD_READ ("audio");
    buffer += input.value;
    while(buffer.size() < frame->nb_samples * soundio_get_bytes_per_frame(sample_format, n_channels)) {
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	AGAIL::RecordObject input = myMicrophone->AGAIL_DEVICE_METHOD_READ ("audio");
	buffer += input.value;
    }

    memcpy(frame->data[0], buffer.c_str(), frame->nb_samples * soundio_get_bytes_per_frame(sample_format, n_channels));
    // Update the buffer and the frame timestamp
    buffer.erase(0, frame->nb_samples * soundio_get_bytes_per_frame(sample_format, n_channels));
    frame->pts = out_audio_stream.next_pts;
    out_audio_stream.next_pts  += frame->nb_samples;

    return frame;
}

// Writes the packet to the output audio stream
int InAudioStream::write_frame(const AVRational *time_base, AVPacket *out_pkt)
{
	int ret;

    // Rescale output packet timestamp values from codec to stream timebase
    av_packet_rescale_ts(out_pkt, *time_base, out_audio_stream.st->time_base);
    out_pkt->stream_index = out_audio_stream.st->index;
    // Write the compressed frame to the output audio stream
    //log_packet(out_pkt);
    if((ret = av_interleaved_write_frame(out_format_ctx, out_pkt)) < 0)
    {
    	fprintf(stderr, "Error while writing audio frame (%s)\n", av_err2str(ret));
    	return 1;
    }

    return 0;
}

// Prints a log message about the packet being written to the output audio stream
void InAudioStream::log_packet(const AVPacket *out_pkt)
{
    AVRational *time_base = &out_format_ctx->streams[out_pkt->stream_index]->time_base;
    fprintf(stderr, "pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           av_ts2str(out_pkt->pts), av_ts2timestr(out_pkt->pts, time_base),
           av_ts2str(out_pkt->dts), av_ts2timestr(out_pkt->dts, time_base),
           av_ts2str(out_pkt->duration), av_ts2timestr(out_pkt->duration, time_base),
		   out_pkt->stream_index);
}

// Sets the output stream sample format according to the input stream sample format
AVSampleFormat InAudioStream::get_sample_format(){
	AVSampleFormat out_sample_format;

	switch(sample_format){
	case SoundIoFormatU8:
		out_sample_format = AV_SAMPLE_FMT_U8;
		break;
	case SoundIoFormatS16NE:
		out_sample_format = AV_SAMPLE_FMT_S16;
		break;
	case SoundIoFormatS32NE:
		out_sample_format = AV_SAMPLE_FMT_S32;
		break;
	case SoundIoFormatFloat32NE:
		out_sample_format = AV_SAMPLE_FMT_FLT;
		break;
	case SoundIoFormatFloat64NE:
		out_sample_format = AV_SAMPLE_FMT_DBL;
		break;
	default:
		fprintf(stderr, "Input sample format not supported\n");
		out_sample_format = AV_SAMPLE_FMT_NONE;
	}

	return out_sample_format;
}
