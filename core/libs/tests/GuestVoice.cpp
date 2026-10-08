#include "SceTypes.hpp"
#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdlib>
#include <exception>

extern "C" {
int APS5_VABI sceVoiceInit(VoiceInitParam*, std::int32_t);
int APS5_VABI sceVoiceCreatePort(std::uint32_t*, const VoicePortParam*);
int APS5_VABI sceVoiceDeletePort(std::uint32_t);
int APS5_VABI sceVoiceSetMuteFlag(std::uint32_t, bool);
int APS5_VABI sceVoiceGetResourceInfo(VoiceResourceInfo*);
}

static void Require(bool value) { if (!value) std::abort(); }

static bool Throws(std::uint32_t port, bool muted) {
    try { sceVoiceSetMuteFlag(port, muted); } catch (const std::exception&) { return true; }
    return false;
}

static void CheckResourceInfo() {
    static_assert(sizeof(VoiceResourceInfo) == 8);
    Require(sceVoiceGetResourceInfo(nullptr) == static_cast<int>(0x804E0805u));
    VoiceResourceInfo info{0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    Require(sceVoiceGetResourceInfo(&info) == 0);
    Require(info.max_in_voice_port == 2);
    Require(info.max_out_voice_port == 4);
    Require(info.max_in_device_port == 0);
    Require(info.max_out_device_port == 5);
}

int main() {
    CheckResourceInfo();
    Require(Throws(0, true));
    VoiceInitParam init{};
    Require(sceVoiceInit(&init, 0) == 0);
    VoicePortParam param{};
    param.port_type = 4;
    param.volume = 1.0f;
    param.voice.bitrate = 16000;
    std::uint32_t port = 0;
    Require(sceVoiceCreatePort(&port, &param) == 0);
    Require(sceVoiceSetMuteFlag(port, true) == 0);
    Require(sceVoiceSetMuteFlag(port, false) == 0);
    Require(Throws(port + 1, true));
    Require(sceVoiceDeletePort(port) == 0);
    Require(Throws(port, true));
}
