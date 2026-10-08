#include "prx/libSceAgcDriver/State/include/Status.hpp"

#include <cstdint>
#include <cstddef>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

namespace {

constexpr int DebugUnavailable = static_cast<int>(0x8A6C1000u);

}

extern "C" {

bool APS5_VABI sceAgcDriverIsCaptureInProgress(void) {
    return false;
}

bool APS5_VABI sceAgcDriverIsTraceInProgress(void) {
    return false;
}

bool APS5_VABI sceAgcDriverIsSubmitValidationEnabled(void) {
    return false;
}

int APS5_VABI sceAgcDriverRequestCaptureStart(void) {
    return DebugUnavailable;
}

int APS5_VABI sceAgcDriverRequestCaptureStop(void) {
    return DebugUnavailable;
}

int APS5_VABI sceAgcDriverTriggerCapture(void) {
    return DebugUnavailable;
}

}
