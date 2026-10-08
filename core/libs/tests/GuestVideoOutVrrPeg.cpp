#include "SceTypes.hpp"
#include "prx/libc/include/Shutdown.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

extern "C" {
int APS5_VABI sceVideoOutOpen(int userId, int busType, int index, const void* param);
int APS5_VABI sceVideoOutClose(int handle);
int APS5_VABI sceVideoOutGetOutputStatus(int handle, VideoOutOutputStatus* status);
int APS5_VABI sceVideoOutVrrPegToFixedRate(int handle, std::uint64_t arg1, std::uint64_t arg2);
int APS5_VABI sceVideoOutVrrUnpegFromFixedRate(int handle);
}

static constexpr int SYSTEM_USER = 255;
static constexpr int MAIN_BUS = 0;
static constexpr int NEVER_OPENED_HANDLE = 2;

static void Require(bool value) { if (!value) std::abort(); }

static bool PegRejects(int handle) {
    try {
        sceVideoOutVrrPegToFixedRate(handle, 0, 0);
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

static bool UnpegRejects(int handle) {
    try {
        sceVideoOutVrrUnpegFromFixedRate(handle);
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

int main() {
    const int handle = sceVideoOutOpen(SYSTEM_USER, MAIN_BUS, 0, nullptr);
    Require(handle > 0);

    VideoOutOutputStatus before{};
    Require(sceVideoOutGetOutputStatus(handle, &before) == 0);
    Require(sceVideoOutVrrPegToFixedRate(handle, 1, 2) == 0);
    VideoOutOutputStatus pegged{};
    Require(sceVideoOutGetOutputStatus(handle, &pegged) == 0);
    Require(std::memcmp(&before, &pegged, sizeof(before)) == 0);
    Require(sceVideoOutVrrUnpegFromFixedRate(handle) == 0);
    VideoOutOutputStatus unpegged{};
    Require(sceVideoOutGetOutputStatus(handle, &unpegged) == 0);
    Require(std::memcmp(&before, &unpegged, sizeof(before)) == 0);

    for (int invalid : {0, -1, NEVER_OPENED_HANDLE}) {
        Require(PegRejects(invalid));
        Require(UnpegRejects(invalid));
    }

    Require(sceVideoOutClose(handle) == 0);
    Require(PegRejects(handle));
    Require(UnpegRejects(handle));
    LibcRunShutdown_nid_postfix();
}
