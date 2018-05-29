#include "TestDeviceCaller.h"

const std::string AGAIL::TestDeviceCaller::TEST_DEVICE = "test";

AGAIL::TestDeviceCaller::TestDeviceCaller (std::string address) : AGAIL::DeviceCaller (address, BASE_BUS_PATH + "/" + TEST_DEVICE + address) {}

void AGAIL::TestDeviceCaller::AGAIL_TESTDEVICE_METHOD_PROPRIETARY () {
    GVariant* gVar = genericMethod (STR(AGAIL_TESTDEVICE_METHOD_PROPRIETARY),
                                    NULL,
                                    NULL);
}
