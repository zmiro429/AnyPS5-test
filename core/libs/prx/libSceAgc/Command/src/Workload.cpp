#include "prx/libSceAgc/Command/include/Workload.hpp"

#include "prx/libSceAgcDriver/Resource/include/Registration.hpp"
#include <cstdint>

namespace Agc::Command {

namespace {

constexpr std::uint32_t OpcodeSetWorkload = 0x1eu;
constexpr std::uint32_t OpcodeWriteData = 0x37u;
constexpr std::uint32_t OpcodeSetUconfigReg = 0x79u;
constexpr std::uint32_t WorkloadStreamRegister = 0x342u;
constexpr std::uint32_t WorkloadMaskHighRegister = 0xc343u;
constexpr std::uint32_t WriteDataRegisterControl = 0x06010000u;
constexpr std::uint32_t ActivePrefix = 0xcc000000u;
constexpr std::uint32_t CompletePrefix = 0xcd000000u;
constexpr std::uint32_t ActiveControl = 0x267u;
constexpr std::uint32_t CompleteControl = 0x275u;
constexpr std::uint32_t StandaloneControl = 0x40000000u;
constexpr std::uint32_t MaxWorkloadId = 63u;

std::uint64_t StreamSlot(std::uint32_t streamId, const char* function) {
    std::uint64_t address = 0;
    Require(AgcDriverGetWorkloadStreamSlot_nid_postfix(streamId, &address), function, "workload stream is not registered");
    return address;
}

std::uint32_t Flags(bool standalone) {
    return standalone ? 1u << 2u : 0u;
}

std::uint32_t Control(std::uint32_t control, bool standalone) {
    return standalone ? control | StandaloneControl : control;
}

void WriteStreamMask(std::uint32_t* packet, bool standalone, std::uint32_t streamId, std::uint64_t mask) {
    packet[0] = Header(OpcodeSetUconfigReg, 4, Flags(standalone));
    packet[1] = WorkloadStreamRegister;
    packet[2] = ActivePrefix | streamId;
    packet[3] = static_cast<std::uint32_t>(mask);
    packet[4] = Header(OpcodeWriteData, 5, Flags(standalone));
    packet[5] = WriteDataRegisterControl;
    packet[6] = WorkloadMaskHighRegister;
    packet[7] = static_cast<std::uint32_t>(mask >> 32u);
    packet[8] = 0;
}

void WriteSetWorkload(std::uint32_t* packet, std::uint32_t control, std::uint64_t slot, std::uint64_t mask) {
    packet[0] = Header(OpcodeSetWorkload, 9);
    packet[1] = control;
    packet[2] = static_cast<std::uint32_t>(slot);
    packet[3] = static_cast<std::uint32_t>(slot >> 32u);
    packet[4] = static_cast<std::uint32_t>(mask);
    packet[5] = static_cast<std::uint32_t>(mask >> 32u);
    packet[6] = 0;
    packet[7] = 0;
    packet[8] = 0;
}

}

std::uint32_t* WriteWorkloadsActive(CommandBuffer* buffer, bool standalone, std::uint32_t streamId, const std::uint32_t* workloadIds, std::uint32_t workloadCount, const char* function) {
    Require(buffer != nullptr, function, "null command buffer");
    Require(workloadIds != nullptr && workloadCount >= 1 && workloadCount <= MaxWorkloadId, function, "invalid workload list");
    const auto slot = StreamSlot(streamId, function);
    std::uint64_t mask = 0;
    for (std::uint32_t i = 0; i < workloadCount; ++i) {
        Require(workloadIds[i] <= MaxWorkloadId, function, "workload id out of range");
        const auto bit = std::uint64_t{1} << workloadIds[i];
        Require((mask & bit) == 0, function, "repeated workload id");
        mask |= bit;
    }
    auto* packet = Allocate(buffer, 18, function);
    WriteStreamMask(packet, standalone, streamId, mask);
    WriteSetWorkload(packet + 9, Control(ActiveControl, standalone), slot, mask);
    return packet;
}

std::uint32_t* WriteWorkloadComplete(CommandBuffer* buffer, bool standalone, std::uint32_t streamId, std::uint32_t workloadId, const char* function) {
    Require(buffer != nullptr, function, "null command buffer");
    const auto slot = StreamSlot(streamId, function);
    Require(workloadId <= MaxWorkloadId, function, "workload id out of range");
    auto* packet = Allocate(buffer, 12, function);
    packet[0] = Header(OpcodeSetUconfigReg, 3, Flags(standalone));
    packet[1] = WorkloadStreamRegister;
    packet[2] = CompletePrefix | (workloadId << 5u) | streamId;
    WriteSetWorkload(packet + 3, Control(CompleteControl, standalone), slot, ~(std::uint64_t{1} << workloadId));
    return packet;
}

std::uint32_t* WriteWorkloadStreamInactive(CommandBuffer* buffer, bool standalone, std::uint32_t streamId, const char* function) {
    Require(buffer != nullptr, function, "null command buffer");
    StreamSlot(streamId, function);
    auto* packet = Allocate(buffer, 9, function);
    WriteStreamMask(packet, standalone, streamId, 0);
    return packet;
}

}
