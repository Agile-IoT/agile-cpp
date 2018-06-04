#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>

#include "MicrophoneCaller.h"

int main() {
    // Create microphone object
    AGAIL::MicrophoneCaller myMicrophone("0000");
    if (!myMicrophone.isConnected()) {
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

    // Read 5 seconds of the input from the microphone and write it to the string
    int recordTime = 5;
    std::string fileString;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end;
    do {
	std::this_thread::sleep_for (std::chrono::milliseconds (20));
	AGAIL::RecordObject input = myMicrophone.AGAIL_DEVICE_METHOD_READ ("audio");
	fileString += input.value;
	end = std::chrono::steady_clock::now();
    } while (std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() < recordTime);

    // Disconnect from microphone
    myMicrophone.AGAIL_DEVICE_METHOD_DISCONNECT();
    std::cout << "Disconnected from microphone" << std::endl;

    // Record string to file
    std::ofstream recordFile ("record.raw", std::ios::out|std::ios::binary);
    if (!recordFile.is_open()) {
        std::cout << "Error opening the file to write" << std::endl;
        return 1;
    }
    recordFile.write (fileString.c_str(), fileString.size());
    recordFile.close();

    return 0;
}
