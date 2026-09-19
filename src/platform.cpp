#include <liara/abi_version.h>
#include <liara/modules.h>
#include <liara/platform/config.h>
#include <liara/platform/platform.h>

#include <cstdint>

static constexpr liara_module_info_t LIARA_PLATFORM_MODULE_INFO = {
    .struct_version = LIARA_MODULE_INFO_VERSION,
    .abi_version = LIARA_ABI_VERSION,
    .module_version = LIARA_PLATFORM_VERSION,
    .reserved = 0,
    .module_name = "Platform",
    .abi_version_str = LIARA_ABI_VERSION_STR,
    .module_version_str = LIARA_PLATFORM_VERSION_STRING,
};

const liara_module_info_t* liara_platform_info(void) { return &LIARA_PLATFORM_MODULE_INFO; }

uint32_t liara_platform_abi_version(void) { return LIARA_PLATFORM_MODULE_INFO.abi_version; }
