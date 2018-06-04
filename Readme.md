# agile-cpp

This repository contains a partial C++ implementation of the AGILE stack (focusing on the Device component).

## AGILE D-Bus C++ interface

Contains the C++ mapping of the AGILE D-Bus interface. The code can be found in directory **org.eclipse.agail**.

## AGILE Device

Exposes the org.eclipse.agail.Device interface.

* D-Bus interface name **org.eclipse.agail.Device**

* D-Bus interface path **/org/eclipse/agail/Device**

The code can be found in directory **org.eclipse.agail.Device**.

### Examples

* Test Device: the code in files TestDevice.h and TestDevice.cpp implements a fake AGILE device, and can be used as a simple reference on how to implement new AGILE devices in C++.

* Microphone: the code in files Microphone.h and Microphone.cpp implements an AGILE microphone, whose D-Bus path is **/org/eclipse/agail/Device/microphone<microphone-address>**.

* Speaker: the code in files Speaker.h and Speaker.cpp implements an AGILE speaker, whose D-Bus path is **/org/eclipse/agail/Device/speaker<speaker-address>**.

## AGILE Device Caller

Implements an API to allow applications access AGILE devices in a simple way. The code can be found in directory **org.eclipse.agail.DeviceCaller**.

### Examples

* Test Device Caller: the code in files TestDeviceCaller.h and TestDeviceCaller.cpp implements an API to allow applications access Test Device, and can be used as a simple reference on how to implement APIs for new AGILE devices in C++.

* Microphone Caller: the code in files MicrophoneCaller.h and MicrophoneCaller.cpp allows applications to access the microphone through the AGILE API.

* Speaker Caller: the code in files SpeakerCaller.h and SpeakerCaller.cpp allows applications to access the speaker through the AGILE API.

## AGILE C++ example tests

Example programs to test the different AGILE devices. The code can be found in directory **org.eclipse.agail.test**.

* Test Device examples: creates a Test Device object and tests the different AGILE interface methods using the Test Device Caller.

* Microphone example: creates a Microphone object and records 5 seconds of the incoming audio to file record.raw.

* Speaker example: creates a Speaker object and plays the audio in file record.raw.

* Audio example: creates a Microphone and a Speaker object and plays through the speaker the audio coming from the microphone.

## Compiling and running the example tests

### Test Device example

To execute this example, no extra libraries are required. Once the code in the agile-cpp repository has been downloaded, go to the repository root folder and run:

```
cmake .
make
```

The executable files will have been created in the **bin** directory. In order to run them, execute, in two different terminals:

```
bin/testServer
```

```
bin/testClient
```

### Microphone example

The Microphone device depends on the libsoundio library. In Ubuntu or similar systems, this library can be installed by running the command:

```
sudo apt-get install libsoundio-dev
```

Otherwise, the library is available in the corresponding [Github repository](https://github.com/andrewrk/libsoundio).

In order to build the Microphone device, you need to tell it to CMake:

```
cmake -DBUILD_MICROPHONE=ON .
make
```

Then, to run the example:

```
bin/testMicrophoneServer
```

```
bin/testMicrophoneClient
```

### Speaker example

Similar to the Microphone, the Speaker device requires the libsoundio library. In order to build the example:

```
cmake -DBUILD_SPEAKER=ON .
make
```

Then, to run it:

```
bin/testSpeakerServer
```

```
bin/testSpeakerClient
```

### Audio example

Since this example uses the Microphone and Speaker devices, the libsoundio library is required. In order to build the example:

```
cmake -DBUILD_MICROPHONE=ON -DBUILD_SPEAKER=ON .
make
```

Then, to run it:

```
bin/testAudioServer
```

```
bin/testAudioClient
```
