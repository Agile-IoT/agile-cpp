#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <cmath>

#include "SpeakerCaller.h"

int main() {
    // Create speaker object
    AGAIL::SpeakerCaller mySpeaker("0000");
    if (!mySpeaker.isConnected()) {
	std::cout << "Could not connect to D-Bus" << std::endl;
	return 1;
    }

    // Read file to memory
    std::ifstream playFile ("record.raw", std::ios::in|std::ios::binary|std::ios::ate);
    if (!playFile.is_open()) {
	std::cout << "Error opening the file to read" << std::endl;
	return 1;
    }
    std::streampos fileSize = playFile.tellg();
    char* fileBlock = new char [fileSize];
    playFile.seekg (0, std::ios::beg);
    playFile.read (fileBlock, fileSize);
    playFile.close();

    // Connect to speaker
    SoundIoFormat sample_format = SoundIoFormatS16NE;
    int sample_rate = 48000;
    int n_channels = 2;
    mySpeaker.AGAIL_DEVICE_METHOD_WRITE ("sample_format", std::to_string ((int)sample_format));
    mySpeaker.AGAIL_DEVICE_METHOD_WRITE ("sample_rate", std::to_string (sample_rate));
    mySpeaker.AGAIL_DEVICE_METHOD_WRITE ("n_channels", std::to_string (n_channels));
    mySpeaker.AGAIL_DEVICE_METHOD_CONNECT();
    std::cout << "Connected to speaker" << std::endl;
    std::cout << "Output audio device stream: " << soundio_channel_layout_get_default(n_channels)->name << " " << sample_rate << "Hz " << soundio_format_string(sample_format) << " interleaved" << std::endl;

    // Read the content of the file and send it to the speaker
    int period = 20;
    int periodBytes = (int)round (soundio_get_bytes_per_second (sample_format, n_channels, sample_rate) * period / 1000);
    char* fileEnd = fileBlock + fileSize;
    char* curPos = fileBlock;
    char* nextPos = curPos + periodBytes;
    std::chrono::steady_clock::time_point nextTime = std::chrono::steady_clock::now();
    while (nextPos < fileEnd) {
	mySpeaker.AGAIL_DEVICE_METHOD_WRITE ("audio", std::string (curPos, periodBytes));
	curPos = nextPos;
	nextPos += periodBytes;
	nextTime += std::chrono::milliseconds (period);
	std::this_thread::sleep_until (nextTime);
    }

    // Disconnect from speaker and delete file content
    mySpeaker.AGAIL_DEVICE_METHOD_DISCONNECT();
    std::cout << "Disconnected from speaker" << std::endl;
    delete[] fileBlock;

    return 0;
}
