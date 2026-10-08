#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C" {
int APS5_VABI sceNpCommerceDialogInitialize();
int APS5_VABI sceNpCommerceDialogOpen2(const void*);
int APS5_VABI sceNpCommerceDialogUpdateStatus(void);
int APS5_VABI sceNpCommerceDialogGetResult(void*);
int APS5_VABI sceNpCommerceDialogTerminate();
}

static void Require(bool value) { if (!value) std::abort(); }

int main() {
    constexpr int notInitialized = static_cast<int>(0x80B80003u);
    constexpr int argNull = static_cast<int>(0x80B80009u);
    constexpr int notFinished = static_cast<int>(0x80B80006u);
    std::uint8_t param[0x88]{};
    std::uint8_t result[0x30];
    Require(sceNpCommerceDialogOpen2(param) == notInitialized);
    Require(sceNpCommerceDialogInitialize() == 0);
    Require(sceNpCommerceDialogGetResult(result) == notFinished);
    Require(sceNpCommerceDialogOpen2(nullptr) == argNull);
    Require(sceNpCommerceDialogUpdateStatus() == 1);
    Require(sceNpCommerceDialogOpen2(param) == 0);
    Require(sceNpCommerceDialogUpdateStatus() == 3);
    std::memset(result, 0xAA, sizeof(result));
    Require(sceNpCommerceDialogGetResult(result) == 0);
    std::int32_t value = 0;
    std::memcpy(&value, result, sizeof(value));
    Require(value == 1);
    Require(result[4] == 0);
    Require(sceNpCommerceDialogTerminate() == 0);
    Require(sceNpCommerceDialogOpen2(param) == notInitialized);
}
