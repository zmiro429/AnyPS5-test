#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

extern "C" {
int APS5_VABI sceCoredumpAttachUserFile(std::uint32_t, const char*);
int APS5_VABI sceCoredumpAttachMemoryRegionAsUserFile(std::uint32_t, const void*, std::size_t, const char*);
int APS5_VABI sceCoredumpAttachMemoryRegion(std::uint32_t, const void*, std::size_t);
int APS5_VABI sceCoredumpAttachUserMemoryFile(std::uint32_t, const void*, std::size_t);
}

static void Require(bool value) { if (!value) std::abort(); }

int main() {
    constexpr int notInHandler = static_cast<int>(0x81180003u);
    const std::uint8_t memory[16]{};
    Require(sceCoredumpAttachUserFile(1, "/app0/log.txt") == notInHandler);
    Require(sceCoredumpAttachMemoryRegionAsUserFile(2, memory, sizeof(memory), "memory") == notInHandler);
    Require(sceCoredumpAttachMemoryRegion(3, memory, sizeof(memory)) == notInHandler);
    Require(sceCoredumpAttachUserMemoryFile(4, memory, sizeof(memory)) == notInHandler);
}
