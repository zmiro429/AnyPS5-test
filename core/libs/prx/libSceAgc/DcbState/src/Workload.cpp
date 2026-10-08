#include "prx/libSceAgc/DcbState/include/Workload.hpp"

#include "prx/libSceAgc/Command/include/Packet.hpp"
#include "prx/libSceAgc/Command/include/Workload.hpp"
#include <cstdint>
#include <cstddef>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

extern "C" {

uint32_t* APS5_VABI sceAgcDcbSetWorkloadComplete(CommandBuffer* buf, uint32_t stream_id, uint32_t workload_id) {
    return Agc::Command::WriteWorkloadComplete(buf, true, stream_id, workload_id, __func__);
}

uint32_t* APS5_VABI sceAgcDcbSetWorkloadsActive(CommandBuffer* buf, uint32_t stream_id, const uint32_t* workload_ids, uint32_t workload_count) {
    return Agc::Command::WriteWorkloadsActive(buf, true, stream_id, workload_ids, workload_count, __func__);
}

uint32_t* APS5_VABI sceAgcDcbSetWorkloadStreamInactive(CommandBuffer* buf, uint32_t stream_id) {
    return Agc::Command::WriteWorkloadStreamInactive(buf, true, stream_id, __func__);
}

}
