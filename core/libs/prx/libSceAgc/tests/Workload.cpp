#include "SceTypes.hpp"
#include "prx/libc/include/general/VabiMacros.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

extern "C" {
int APS5_VABI sceAgcDriverRegisterWorkloadStream(std::uint32_t, const void*);
int APS5_VABI sceAgcDriverUnregisterWorkloadStream(std::uint32_t);
bool AgcDriverGetWorkloadStreamSlot_nid_postfix(std::uint32_t, std::uint64_t*);
std::uint32_t* APS5_VABI sceAgcDcbSetWorkloadsActive(CommandBuffer*, std::uint32_t, const std::uint32_t*, std::uint32_t);
std::uint32_t* APS5_VABI sceAgcDcbSetWorkloadComplete(CommandBuffer*, std::uint32_t, std::uint32_t);
std::uint32_t* APS5_VABI sceAgcDcbSetWorkloadStreamInactive(CommandBuffer*, std::uint32_t);
std::uint32_t* APS5_VABI sceAgcAcbSetWorkloadsActive(CommandBuffer*, std::uint32_t, const std::uint32_t*, std::uint32_t);
std::uint32_t* APS5_VABI sceAgcAcbSetWorkloadComplete(CommandBuffer*, std::uint32_t, std::uint32_t);
std::uint32_t* APS5_VABI sceAgcAcbSetWorkloadStreamInactive(CommandBuffer*, std::uint32_t);
}

namespace {

constexpr std::uint32_t Stream = 5;

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

template <typename TAction>
void expectFailure(TAction action) {
    try {
        action();
    } catch (const std::runtime_error&) {
        return;
    }
    throw std::runtime_error("expected invalid input to fail");
}

struct Storage {
    std::array<std::uint32_t, 32> words{};
    CommandBuffer buffer{words.data(), words.data() + words.size(), words.data(), words.data() + words.size(), nullptr, nullptr, 0};

    Storage() { words.fill(0xabcdef01u); }
};

template <std::size_t TCount>
void expectPacket(const Storage& storage, const std::uint32_t* packet, const std::array<std::uint32_t, TCount>& expected, const char* message) {
    check(packet == storage.words.data() && std::equal(expected.begin(), expected.end(), packet), message);
    check(storage.buffer.cursor_up == packet + TCount && packet[TCount] == 0xabcdef01u, message);
}

std::uint64_t slotAddress() {
    std::uint64_t slot = 0;
    check(AgcDriverGetWorkloadStreamSlot_nid_postfix(Stream, &slot), "registered stream has no slot");
    return slot;
}

void testActive(bool dcb) {
    Storage storage;
    const std::array<std::uint32_t, 3> ids{0u, 3u, 40u};
    auto* packet = dcb ? sceAgcDcbSetWorkloadsActive(&storage.buffer, Stream, ids.data(), 3) : sceAgcAcbSetWorkloadsActive(&storage.buffer, Stream, ids.data(), 3);
    const auto slot = slotAddress();
    const std::uint32_t flags = dcb ? 4u : 0u;
    const std::array<std::uint32_t, 18> expected{
        0xc0027900u | flags, 0x342u, 0xcc000005u, 0x9u,
        0xc0033700u | flags, 0x06010000u, 0xc343u, 0x100u, 0u,
        0xc0071e00u, dcb ? 0x40000267u : 0x267u, static_cast<std::uint32_t>(slot), static_cast<std::uint32_t>(slot >> 32u), 0x9u, 0x100u, 0u, 0u, 0u};
    expectPacket(storage, packet, expected, "workloads active packet");
}

void testComplete(bool dcb) {
    Storage storage;
    auto* packet = dcb ? sceAgcDcbSetWorkloadComplete(&storage.buffer, Stream, 7) : sceAgcAcbSetWorkloadComplete(&storage.buffer, Stream, 7);
    const auto slot = slotAddress();
    const std::array<std::uint32_t, 12> expected{
        0xc0017900u | (dcb ? 4u : 0u), 0x342u, 0xcd0000e5u,
        0xc0071e00u, dcb ? 0x40000275u : 0x275u, static_cast<std::uint32_t>(slot), static_cast<std::uint32_t>(slot >> 32u), 0xffffff7fu, 0xffffffffu, 0u, 0u, 0u};
    expectPacket(storage, packet, expected, "workload complete packet");
}

void testInactive(bool dcb) {
    Storage storage;
    auto* packet = dcb ? sceAgcDcbSetWorkloadStreamInactive(&storage.buffer, Stream) : sceAgcAcbSetWorkloadStreamInactive(&storage.buffer, Stream);
    const std::uint32_t flags = dcb ? 4u : 0u;
    const std::array<std::uint32_t, 9> expected{0xc0027900u | flags, 0x342u, 0xcc000005u, 0u, 0xc0033700u | flags, 0x06010000u, 0xc343u, 0u, 0u};
    expectPacket(storage, packet, expected, "workload stream inactive packet");
}

void testRejections() {
    Storage storage;
    const std::array<std::uint32_t, 2> repeated{4u, 4u};
    const std::array<std::uint32_t, 1> outOfRange{64u};
    expectFailure([&] { sceAgcDcbSetWorkloadsActive(&storage.buffer, Stream, nullptr, 1); });
    expectFailure([&] { sceAgcDcbSetWorkloadsActive(&storage.buffer, Stream, repeated.data(), 0); });
    expectFailure([&] { sceAgcDcbSetWorkloadsActive(&storage.buffer, Stream, repeated.data(), 64); });
    expectFailure([&] { sceAgcDcbSetWorkloadsActive(&storage.buffer, Stream, repeated.data(), 2); });
    expectFailure([&] { sceAgcAcbSetWorkloadsActive(&storage.buffer, Stream, outOfRange.data(), 1); });
    expectFailure([&] { sceAgcDcbSetWorkloadsActive(nullptr, Stream, repeated.data(), 1); });
    expectFailure([&] { sceAgcDcbSetWorkloadComplete(&storage.buffer, Stream, 64); });
    for (std::uint32_t stream : {0u, 6u, 32u}) {
        expectFailure([&] { sceAgcDcbSetWorkloadsActive(&storage.buffer, stream, repeated.data(), 1); });
        expectFailure([&] { sceAgcAcbSetWorkloadComplete(&storage.buffer, stream, 1); });
        expectFailure([&] { sceAgcDcbSetWorkloadStreamInactive(&storage.buffer, stream); });
    }
    check(storage.buffer.cursor_up == storage.words.data() && storage.words[0] == 0xabcdef01u, "a rejected workload packet was written");
}

void testRegistry() {
    const std::uint8_t descriptor[32]{};
    std::uint64_t slot = 0;
    check(!AgcDriverGetWorkloadStreamSlot_nid_postfix(Stream, &slot), "unregistered stream has a slot");
    check(sceAgcDriverRegisterWorkloadStream(Stream, descriptor) == 0, "stream registration failed");
    check(sceAgcDriverRegisterWorkloadStream(Stream, descriptor) == static_cast<int>(0x8A6C0033u), "duplicate stream accepted");
    check(sceAgcDriverRegisterWorkloadStream(0, descriptor) == static_cast<int>(0x8A6C0033u), "system stream accepted");
    check(sceAgcDriverRegisterWorkloadStream(32, descriptor) == static_cast<int>(0x8A6C0033u), "stream 32 accepted");
    check(sceAgcDriverRegisterWorkloadStream(6, nullptr) == static_cast<int>(0x8A6C0035u), "null stream accepted");
    check(sceAgcDriverUnregisterWorkloadStream(6) == static_cast<int>(0x8A6C003Au), "unregistered stream removed");
    check(AgcDriverGetWorkloadStreamSlot_nid_postfix(Stream, &slot) && slot % 8 == 0, "registered stream slot");
    check(*reinterpret_cast<const std::uint64_t*>(static_cast<std::uintptr_t>(slot)) == ~std::uint64_t{0}, "stream slot is not cleared to all ones");
    std::uint64_t first = 0;
    check(sceAgcDriverRegisterWorkloadStream(1, descriptor) == 0 && AgcDriverGetWorkloadStreamSlot_nid_postfix(1, &first), "stream 1 registration failed");
    check(slot == first + 4 * sizeof(std::uint64_t), "stream slots are not consecutive");
    check(*reinterpret_cast<const std::uint64_t*>(static_cast<std::uintptr_t>(first)) == 0, "stream 1 slot is not zeroed");
    check(sceAgcDriverUnregisterWorkloadStream(1) == 0, "stream 1 unregistration failed");
}

}

int main() {
    try {
        testRegistry();
        for (bool dcb : {true, false}) {
            testActive(dcb);
            testComplete(dcb);
            testInactive(dcb);
        }
        testRejections();
        check(sceAgcDriverUnregisterWorkloadStream(Stream) == 0, "stream unregistration failed");
        Storage storage;
        expectFailure([&] { sceAgcDcbSetWorkloadStreamInactive(&storage.buffer, Stream); });
        std::puts("AGC workload tests passed");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
