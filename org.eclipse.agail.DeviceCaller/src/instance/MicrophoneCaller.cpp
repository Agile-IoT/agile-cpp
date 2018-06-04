#include "MicrophoneCaller.h"

const std::string AGAIL::MicrophoneCaller::MICROPHONE_DEVICE = "microphone";

AGAIL::MicrophoneCaller::MicrophoneCaller (std::string address) : AGAIL::DeviceCaller (address, BASE_BUS_PATH + "/" + MICROPHONE_DEVICE + address) {}
