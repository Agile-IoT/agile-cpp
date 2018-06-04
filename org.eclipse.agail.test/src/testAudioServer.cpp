#include <thread>

#include "Microphone.h"
#include "Speaker.h"

void microphoneThreadFunc ();
void speakerThreadFunc ();

int main () {
    std::thread microphoneThread (microphoneThreadFunc);
    std::thread speakerThread (speakerThreadFunc);

    microphoneThread.join();
    speakerThread.join();

    return 0;
}

void microphoneThreadFunc () {
    AGAIL::Microphone ("Default microphone", "0000");
}

void speakerThreadFunc () {
    AGAIL::Speaker ("Default speaker", "0000");
}
