# agile-cpp

This repository contains a partial C++ implementation of the AGILE stack (focusing on the Device component).

## AGILE D-Bus C++ interface

Contains the C++ mapping of the AGILE D-Bus interface. The code can be found in directory **org.eclipse.agail**.

## AGILE Device

Exposes the org.eclipse.agail.Device interface.

* D-Bus interface name **org.eclipse.agail.Device**

* D-Bus interface path **/org/eclipse/agail/Device**

The code can be found in directory **org.eclipse.agail.Device**.

### Example

* Test Device: the code in files TestDevice.h and TestDevice.cpp implements a fake AGILE device, and can be used as a simple reference on how to implement new AGILE devices.

## AGILE Device Caller

Implements an API to allow applications access AGILE devices in a simple way. The code can be found in directory **org.eclipse.agail.DeviceCaller**.

### Example

* Test Device Caller: the code in files TestDeviceCaller.h and TestDeviceCaller.cpp implements an API to allow applications access Test Device, and can be used as a simple reference on how to implement APIs for new AGILE devices.

## AGILE C++ example test

Creates a Test Device object and tests the different AGILE interface methods using the Test Device Caller. The code can be found in directory **org.eclipse.agail.test**.

## Compiling and running the example test

Once the code in the repository has been downloaded, go to the repository root folder and run:

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
