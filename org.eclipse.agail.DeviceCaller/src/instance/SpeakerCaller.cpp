#include "SpeakerCaller.h"

const std::string AGAIL::SpeakerCaller::SPEAKER_DEVICE = "speaker";

AGAIL::SpeakerCaller::SpeakerCaller (std::string address) : AGAIL::DeviceCaller (address, BASE_BUS_PATH + "/" + SPEAKER_DEVICE + address) {}
