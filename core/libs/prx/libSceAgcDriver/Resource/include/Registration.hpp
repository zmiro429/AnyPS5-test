#ifndef CORE_LIBS_PRX_LIBSCEAGCDRIVER_RESOURCE_INCLUDE_REGISTRATION_HPP
#define CORE_LIBS_PRX_LIBSCEAGCDRIVER_RESOURCE_INCLUDE_REGISTRATION_HPP

#include <cstdint>

extern "C" bool AgcDriverGetWorkloadStreamSlot_nid_postfix(std::uint32_t streamId, std::uint64_t* address);

#endif
