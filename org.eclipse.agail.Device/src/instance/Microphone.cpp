#include "Microphone.h"

#include <chrono>
#include <cstring>

#include <iostream> // TODO: substitute by log library

const std::string AGAIL::Microphone::MICROPHONE_DEVICE = "microphone";
const std::string AGAIL::Microphone::MICROPHONE_PROTOCOL = "org.eclipse.agail.protocol.microphone";

AGAIL::Microphone::Microphone (std::string deviceName, std::string address) : DeviceImp (deviceName, MICROPHONE_PROTOCOL, address, BASE_BUS_PATH + "/" + MICROPHONE_DEVICE + address, std::list<AGAIL::DeviceComponent> ({AGAIL::DeviceComponent ("audio", ""), AGAIL::DeviceComponent ("sample_format",""), AGAIL::DeviceComponent ("sample_rate", ""), AGAIL::DeviceComponent ("n_channels", "")})) {
    dbusConnect (deviceAgileID, path, generateDbusIntrospection (""));
}

AGAIL::Microphone::Microphone (AGAIL::DeviceDefinition definition) : DeviceImp (definition) {
    dbusConnect (deviceAgileID, path, generateDbusIntrospection (""));
}

/////////////////////////////////////
// AGILE::Device interface methods //
/////////////////////////////////////

void AGAIL::Microphone::AGAIL_DEVICE_METHOD_CONNECT () {
    if (isConnected) {
        std::cout << "Microphone already connected" << std::endl;
        return;
    }

    int err;
    std::cout << "Connecting to microphone..." << std::endl;
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
    // Connect to default input device
    int default_in_device_index = soundio_default_input_device_index (soundio);
    if (default_in_device_index < 0) {
        fprintf (stderr, "No input device found\n");
        return;
    }
    device = soundio_get_input_device (soundio, default_in_device_index);
    if (!device) {
        fprintf (stderr, "Could not connect to default input device. Out of memory\n");
        return;
    }
    fprintf (stderr, "Input device: %s\n", device->name);
    if (device->probe_error) {
        fprintf (stderr, "Unable to probe input device (%s)\n", soundio_strerror(device->probe_error));
        return;
    }
    // Create input stream and set parameters
    instream = soundio_instream_create (device);
    if (instream->format == SoundIoFormatInvalid)
        instream->format = device->formats[0];
    for (int i = 0; i < device->format_count; i++)
        if (device->formats[i] == SoundIoFormatS16NE)
            instream->format = SoundIoFormatS16NE;
    instream->read_callback = read_callback;
    instream->userdata = &(ring_buffer);
    instream->software_latency = 0.02; // otherwise, the microphone latency may be up to 2 seconds
    // Open input stream device
    if ((err = soundio_instream_open (instream))) {
        fprintf (stderr, "Unable to open input device (%s)\n", soundio_strerror(err));
                return;
    }
    if (instream->layout_error)
        fprintf (stderr, "Unable to set channel layout (%s)\n", soundio_strerror(instream->layout_error));
    fprintf (stderr, "Input audio device stream: %s %dHz %s interleaved\n",
            instream->layout.name, instream->sample_rate, soundio_format_string (instream->format));
    // Create ring buffer (buffer where we write the input frames and from which we read those frames for the output)
    const int ring_buffer_duration_seconds = 5;
    int capacity = ring_buffer_duration_seconds * instream->sample_rate * instream->bytes_per_frame;
    ring_buffer = soundio_ring_buffer_create (soundio, capacity);
    if (!ring_buffer) {
        fprintf (stderr, "Could not create ring buffer. Out of memory\n");
        return;
    }
    // Start input stream device
    if ((err = soundio_instream_start (instream))) {
        fprintf (stderr, "Unable to start input device (%s)\n", soundio_strerror(err));
        return;
    }

    sample_format = instream->format;
    sample_rate = instream->sample_rate;
    n_channels = instream->layout.channel_count;
    isConnected = true;
    std::cout << "Connected to microphone" << std::endl;
}

void AGAIL::Microphone::AGAIL_DEVICE_METHOD_DISCONNECT () {
    isConnected = false;
    soundio_instream_destroy (instream);
    soundio_device_unref (device);
    soundio_destroy (soundio);
    std::cout << "Disconnected from microphone" << std::endl;
}

AGAIL::RecordObject AGAIL::Microphone::AGAIL_DEVICE_METHOD_READ (std::string componentID) {
    if (!isConnected) {
	std::cout << "Microphone not connected. Unable to perform read operation" << std::endl;
	return RecordObject ("", "", "", "", "", 0);
    }

    if (componentID == "sample_format") {
        return RecordObject (deviceID, componentID, std::to_string ((int)sample_format), getMeasurementUnit (componentID), "", std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count());
    }
    else if (componentID == "sample_rate") {
	return RecordObject (deviceID, componentID, std::to_string ((int)sample_rate), getMeasurementUnit (componentID), "", std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count());
    }
    else if (componentID == "n_channels") {
        return RecordObject (deviceID, componentID, std::to_string ((int)n_channels), getMeasurementUnit (componentID), "", std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count());
    }
    else if (componentID == "audio") {
	return readAudio (componentID);
    }
    else {
	std::cout << "Invalid option for reading to microphone" << std::endl;
    }
}

AGAIL::RecordObject AGAIL::Microphone::readAudio (std::string componentID) {
    // Copy the data from the ring buffer to the frame
    char* read_buf = soundio_ring_buffer_read_ptr (ring_buffer);
    int fill_bytes = soundio_ring_buffer_fill_count (ring_buffer);
    std::string value (read_buf, fill_bytes);
    // Update the ring buffer read pointer
    soundio_ring_buffer_advance_read_ptr (ring_buffer, fill_bytes);

    return generateReadRecord (componentID, value);
}

AGAIL::RecordObject AGAIL::Microphone::generateReadRecord (std::string componentID, std::string value) {
    // Generate the record object in data member variable
    if (data != NULL)
        delete data;
    data = new RecordObject (deviceID, componentID, value, getMeasurementUnit (componentID), "", std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count());
    // Update lastReadStore variable
    lastReadStore.erase (componentID);
    lastReadStore.insert (std::map<std::string,AGAIL::RecordObject>::value_type(componentID, *data));

    return *data;
}

void AGAIL::Microphone::read_callback (struct SoundIoInStream *instream, int frame_count_min, int frame_count_max) {
    SoundIoRingBuffer *ring_buffer = (*(SoundIoRingBuffer**)instream->userdata);
    struct SoundIoChannelArea *areas;
    int err;

    // Initialize ring buffer writing variables
    char *write_ptr = soundio_ring_buffer_write_ptr (ring_buffer);
    int free_bytes = soundio_ring_buffer_free_count (ring_buffer);
    int free_frames = free_bytes/instream->bytes_per_frame;

    // Check if there is enough space in the ring buffer to read the input coming from the microphone
    if (free_frames < frame_count_min) {
	fprintf (stderr, "No enough free space in the microphone ring buffer. Ring buffer overflow\n");
	exit (1);
    }

    // Get the frames from the microphone. This is done through an iterative process: for each iteration,
    // we tell the device how many frames we want to read, the device tells us how many we can read
    // in that iteration, we get those frames from a buffer and we tell the device that we have finished
    // reading those frames
    int write_frames = std::min(free_frames, frame_count_max);
    int frames_left = write_frames;
    for (;;) {
	int frame_count = frames_left;
	if ((err = soundio_instream_begin_read(instream, &areas, &frame_count))) {
	    fprintf (stderr, "Error trying to begin reading from the microphone (%s)\n", soundio_strerror(err));
            exit (1);
        }
        if (!frame_count)
            break;
        if (!areas) {
            // Due to an overflow there is a hole. Fill the ring buffer with
	    // silence for the size of the hole.
            memset (write_ptr, 0, frame_count * instream->bytes_per_frame);
        } else {
            memcpy(write_ptr, areas[0].ptr, instream->bytes_per_sample * instream->layout.channel_count * frame_count);
            write_ptr += instream->bytes_per_sample * instream->layout.channel_count * frame_count;
        }
        if ((err = soundio_instream_end_read(instream))) {
            fprintf(stderr, "Error trying to end reading from the microphone (%s)\n", soundio_strerror(err));
	    exit (1);
        }
        frames_left -= frame_count;
        if (frames_left <= 0)
            break;
    }

    // Update the read pointer of the write buffer
    int advance_bytes = write_frames * instream->bytes_per_frame;
    soundio_ring_buffer_advance_write_ptr(ring_buffer, advance_bytes);
}

//////////////////////////////////////////////////////
// Callback functions for g_bus_own_name() function //
//////////////////////////////////////////////////////

void AGAIL::Microphone::onBusAcquiredCb(GDBusConnection *conn, const gchar *name) {
    std::cout << "D-Bus bus acquired by microphone" << std::endl;
}

void AGAIL::Microphone::onNameAcquiredCb(GDBusConnection *conn, const gchar *name) {
    std::cout << "D-bus name acquired by microphone" << std::endl;
}

void AGAIL::Microphone::onNameLostCb(GDBusConnection *conn, const gchar *name) {
    std::cout << "D-bus name lost by microphone" << std::endl;
}
