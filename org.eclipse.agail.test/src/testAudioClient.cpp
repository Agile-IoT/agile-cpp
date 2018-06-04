#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

#include "MicrophoneCaller.h"
#include "SpeakerCaller.h"

int main() {
    // Create microphone and speaker objects
    AGAIL::MicrophoneCaller myMicrophone("0000");
    AGAIL::SpeakerCaller mySpeaker("0000");
    if (!myMicrophone.isConnected() || !mySpeaker.isConnected()) {
        std::cout << "Could not connect to D-Bus" << std::endl;
        return 1;
    }

    // Connect to microphone
    myMicrophone.AGAIL_DEVICE_METHOD_CONNECT();
    std::cout << "Connected to microphone" << std::endl;
    SoundIoFormat sample_format = (SoundIoFormat)std::stoi (myMicrophone.AGAIL_DEVICE_METHOD_READ ("sample_format").value);
    int sample_rate = std::stoi (myMicrophone.AGAIL_DEVICE_METHOD_READ ("sample_rate").value);
    int n_channels = std::stoi (myMicrophone.AGAIL_DEVICE_METHOD_READ ("n_channels").value);
    std::cout << "Input audio device stream: " << soundio_channel_layout_get_default(n_channels)->name << " " << sample_rate << "Hz " << soundio_format_string (sample_format) << " interleaved" << std::endl;

    std::this_thread::sleep_for (std::chrono::milliseconds (100));

    // Connect to speaker
    mySpeaker.AGAIL_DEVICE_METHOD_WRITE ("sample_format", std::to_string ((int)sample_format));
    mySpeaker.AGAIL_DEVICE_METHOD_WRITE ("sample_rate", std::to_string (sample_rate));
    mySpeaker.AGAIL_DEVICE_METHOD_WRITE ("n_channels", std::to_string (n_channels));
    mySpeaker.AGAIL_DEVICE_METHOD_CONNECT();
    std::cout << "Connected to speaker" << std::endl;
    std::cout << "Output audio device stream: " << soundio_channel_layout_get_default(n_channels)->name << " " << sample_rate << "Hz " << soundio_format_string(sample_format) << " interleaved" << std::endl;

    int recordTime = 5;
    int period = 20;
    int periodBytes = (int)round (soundio_get_bytes_per_second (sample_format, n_channels, sample_rate) * period / 1000);
    std::string fileString;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end;
    std::chrono::steady_clock::time_point nextTime = std::chrono::steady_clock::now();
    do {
        AGAIL::RecordObject input = myMicrophone.AGAIL_DEVICE_METHOD_READ ("audio");
        mySpeaker.AGAIL_DEVICE_METHOD_WRITE ("audio", input.value);
	std::this_thread::sleep_until (nextTime);
        end = std::chrono::steady_clock::now();
    } while (std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() < recordTime);


    // Disconnect from microphone and speaker
    myMicrophone.AGAIL_DEVICE_METHOD_DISCONNECT();
    std::cout << "Disconnected from microphone" << std::endl;
    mySpeaker.AGAIL_DEVICE_METHOD_DISCONNECT();
    std::cout << "Disconnected from speaker" << std::endl;

    return 0;
}
