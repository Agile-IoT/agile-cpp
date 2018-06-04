#include "Speaker.h"

#include <cstring>

#include <iostream> // TODO: substitute by log library

const std::string AGAIL::Speaker::SPEAKER_DEVICE = "speaker";
const std::string AGAIL::Speaker::SPEAKER_PROTOCOL = "org.eclipse.agail.protocol.speaker";

AGAIL::Speaker::Speaker (std::string deviceName, std::string address) : DeviceImp (deviceName, SPEAKER_PROTOCOL, address, BASE_BUS_PATH + "/" + SPEAKER_DEVICE + address, std::list<AGAIL::DeviceComponent> ({AGAIL::DeviceComponent ("audio", ""), AGAIL::DeviceComponent ("sample_format",""), AGAIL::DeviceComponent ("sample_rate", ""), AGAIL::DeviceComponent ("n_channels", "")})) {
    dbusConnect (deviceAgileID, path, generateDbusIntrospection (""));
}

AGAIL::Speaker::Speaker (AGAIL::DeviceDefinition definition) : DeviceImp (definition) {
    dbusConnect (deviceAgileID, path, generateDbusIntrospection (""));
}

/////////////////////////////////////
// AGILE::Device interface methods //
/////////////////////////////////////

void AGAIL::Speaker::AGAIL_DEVICE_METHOD_CONNECT () {
    if (isConnected) {
        std::cout << "Speaker already connected" << std::endl;
        return;
    }
    if (sample_format == SoundIoFormatInvalid) {
	std::cout << "Error trying to connect to device. Sample format not set" << std::endl;
	return;
    }
    if (sample_rate == 0) {
	std::cout << "Error trying to connect to device. Sample rate not set" << std::endl;
	return;
    }
    if (n_channels == 0) {
	std::cout << "Error trying to connect to device. Number of channels not set" << std::endl;
	return;
    }

    int err;
    std::cout << "Connecting to speaker..." << std::endl;
    // Create SoundIo context
    soundio = soundio_create();
    if (!soundio) {
        fprintf (stderr, "Could not create SoundIo context. Out of memory\n");
        return;
    }
    // Connect to backend
    if ((err = soundio_connect(soundio))) {
        fprintf (stderr, "Error connecting to backend (%s)\n", soundio_strerror(err));
        return;
    }
    // Update information for all connected devices
    soundio_flush_events (soundio);
    // Connect to default output device
    int default_out_device_index = soundio_default_output_device_index (soundio);
    if (default_out_device_index < 0) {
	fprintf (stderr, "No output device found\n");
	return;
    }
    device = soundio_get_output_device (soundio, default_out_device_index);
    if (!device) {
	fprintf (stderr, "Could not connect to default output device. Out of memory\n");
	return;
    }
    fprintf (stderr, "Output device: %s\n", device->name);
    if (device->probe_error) {
	fprintf (stderr, "Unable to probe output device (%s)\n", soundio_strerror(device->probe_error));
	return;
    }
    // Create output stream and set parameters
    outstream = soundio_outstream_create (device);
    outstream->format = sample_format;
    outstream->sample_rate = sample_rate;
    outstream->write_callback = write_callback;
    outstream->userdata = &(ring_buffer);
    outstream->layout = *soundio_channel_layout_get_default (n_channels);
    // Open output stream device
    if ((err = soundio_outstream_open (outstream))) {
	fprintf (stderr, "Unable to open output device (%s)\n", soundio_strerror(err));
	return;
    }
    if (outstream->layout_error)
	fprintf (stderr, "Unable to set channel layout (%s)\n", soundio_strerror(outstream->layout_error));
    fprintf (stderr, "Output audio device stream: %s %dHz %s interleaved\n",
	    outstream->layout.name, outstream->sample_rate, soundio_format_string(outstream->format));
    // Create ring buffer (buffer where we write the input frames and from which we read those frames for the output)
    const int ring_buffer_duration_seconds = 5;
    int capacity = ring_buffer_duration_seconds * outstream->sample_rate * outstream->bytes_per_frame;
    ring_buffer = soundio_ring_buffer_create (soundio, capacity);
    if (!ring_buffer) {
	fprintf (stderr, "Could not create ring buffer. Out of memory\n");
	return;
    }
    // Start output stream device
    if ((err = soundio_outstream_start (outstream))) {
	fprintf (stderr, "Unable to start output device (%s)\n", soundio_strerror(err));
	return;
    }

    isConnected = true;
    std::cout << "Connected to speaker" << std::endl;
}

void AGAIL::Speaker::AGAIL_DEVICE_METHOD_DISCONNECT () {
    isConnected = false;
    soundio_outstream_destroy (outstream);
    soundio_device_unref (device);
    soundio_destroy (soundio);
    std::cout << "Disconnected from speaker" << std::endl;
}

void AGAIL::Speaker::AGAIL_DEVICE_METHOD_WRITE (std::string componentID, std::string payload) {
    if (componentID == "sample_format") {
	if (isConnected) {
	    std::cout << "Error: cannot set sample format while speaker is connected" << std::endl;
	    return;
	}
	sample_format = (SoundIoFormat)std::stoi (payload);
    }
    else if (componentID == "sample_rate") {
	if (isConnected) {
	    std::cout << "Error: cannot set sample rate while speaker is connected" << std::endl;
	    return;
	}
	sample_rate = std::stoi (payload);
    }
    else if (componentID == "n_channels") {
	if (isConnected) {
	    std::cout << "Error: cannot set number of channels while speaker is connected" << std::endl;
	    return;
	}
	n_channels = std::stoi (payload);
    }
    else if (componentID == "audio") {
	if (!isConnected) {
	    std::cout << "Speaker not connected. Unable to perform write operation" << std::endl;
	    return;
	}
	writeAudio (payload);
    }
    else {
	std::cout << "Invalid option for writing to speaker" << std::endl;
    }
}

void AGAIL::Speaker::writeAudio (std::string payload) {
    // Copy the data from the input to the ring buffer
    char *write_ptr = soundio_ring_buffer_write_ptr (ring_buffer);
    unsigned int free_bytes = soundio_ring_buffer_free_count (ring_buffer);
    memcpy (write_ptr, payload.c_str(), payload.size());
    // Update the ring buffer write pointer
    soundio_ring_buffer_advance_write_ptr (ring_buffer, payload.size());
}

void AGAIL::Speaker::write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) {
    SoundIoRingBuffer *ring_buffer = (*(SoundIoRingBuffer**)outstream->userdata);
    struct SoundIoChannelArea *areas;
    int err;

    // Initialize ring buffer reading variables
    int fill_bytes = soundio_ring_buffer_fill_count (ring_buffer);
    char *read_ptr = soundio_ring_buffer_read_ptr (ring_buffer);
    int fill_frames = fill_bytes / outstream->bytes_per_frame;

    // Send the frames to the audio device. This is done through an iterative process: for each iteration,
    // we tell the device how many frames we want to write, the device tells us how many we can write
    // in that iteration, we copy those frames to a buffer and we tell the device that we have finished
    // writing those frames
    if (fill_frames < frame_count_min || fill_frames == 0) {
	fprintf (stderr, "speaker ring buffer overflow\n");
	//exit(1);
	// If there are no frames available, send silence to the audio device
	const int silence_duration_ms = 50; // we send at least 50 milliseconds of silence to the speaker
	int silence_frames = silence_duration_ms * outstream->sample_rate / 1000;
	int frames_left = std::max (silence_frames,frame_count_min);
	for (;;) {
	    int frame_count = frames_left;
	    if ((err = soundio_outstream_begin_write (outstream, &areas, &frame_count))) {
		fprintf (stderr, "Error trying to begin writing to the audio device (%s)\n", soundio_strerror(err));
		exit(1);
	    }
	    if (!frame_count)
		break;
	   memset (areas[0].ptr, 0, outstream->bytes_per_sample * outstream->layout.channel_count * frame_count);
	    if ((err = soundio_outstream_end_write(outstream))) {
		fprintf (stderr, "Error trying to end writing to the audio device (%s)\n", soundio_strerror(err));
		exit(1);
	    }
	    frames_left -= frame_count;
	    if (frames_left <= 0)
		break;
	}
    }
    else{
	// If there are frames available, send them to the audio device
	int read_frames = std::min (fill_frames, frame_count_max);
	int frames_left = read_frames;
	for (;;) {
	    int frame_count = frames_left;
	    if ((err = soundio_outstream_begin_write (outstream, &areas, &frame_count))) {
		fprintf (stderr, "Error trying to begin writing to the audio device (%s)\n", soundio_strerror(err));
		exit(1);
	    }
	    if (!frame_count)
		break;
	    memcpy (areas[0].ptr, read_ptr, outstream->bytes_per_sample * outstream->layout.channel_count * frame_count);
	    read_ptr += outstream->bytes_per_sample * outstream->layout.channel_count * frame_count;
	    if ((err = soundio_outstream_end_write (outstream))) {
		fprintf (stderr, "Error trying to end writing to the audio device (%s)\n", soundio_strerror(err));
		exit(1);
	    }
	    frames_left -= frame_count;
	    if (frames_left <= 0)
		break;
	}

	// Update the read pointer of the ring buffer
	int advance_bytes = read_frames * outstream->bytes_per_frame;
	soundio_ring_buffer_advance_read_ptr(ring_buffer, advance_bytes);
    }
}

//////////////////////////////////////////////////////
// Callback functions for g_bus_own_name() function //
//////////////////////////////////////////////////////

void AGAIL::Speaker::onBusAcquiredCb(GDBusConnection *conn, const gchar *name) {
    std::cout << "D-Bus bus acquired by speaker" << std::endl;
}

void AGAIL::Speaker::onNameAcquiredCb(GDBusConnection *conn, const gchar *name) {
    std::cout << "D-bus name acquired by speaker" << std::endl;
}

void AGAIL::Speaker::onNameLostCb(GDBusConnection *conn, const gchar *name) {
    std::cout << "D-bus name lost by speaker" << std::endl;
}
