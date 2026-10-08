#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

extern "C" {
int APS5_VABI sceRudpInit_nid_postfix(void*, int);
int APS5_VABI sceRudpGetStatus(void*, std::size_t);
int APS5_VABI sceRudpTerminate(void);
}

static void Require(bool value) { if (!value) std::abort(); }

int main() {
    constexpr int notInitialized = static_cast<int>(0x80770001u);
    std::uint8_t status[0xF9];
    for (auto& byte : status) byte = 0xAA;
    Require(sceRudpGetStatus(status, sizeof(status)) == notInitialized);
    for (std::uint8_t byte : status) Require(byte == 0xAA);
    Require(sceRudpInit_nid_postfix(nullptr, 0) == 0);
    Require(sceRudpGetStatus(nullptr, 0) == 0);
    Require(sceRudpGetStatus(status, 0) == 0);
    for (std::uint8_t byte : status) Require(byte == 0xAA);
    Require(sceRudpGetStatus(status, sizeof(status) - 1) == 0);
    for (std::size_t i = 0; i + 1 < sizeof(status); ++i) Require(status[i] == 0);
    Require(status[sizeof(status) - 1] == 0xAA);
    Require(sceRudpTerminate() == 0);
    Require(sceRudpGetStatus(status, sizeof(status)) == notInitialized);
}
