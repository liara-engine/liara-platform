#include "Shutdown.h"
#include "Time.h"

#include <liara/abi_version.h>
#include <liara/modules.h>
#include <liara/platform/config.h>
#include <liara/platform/platform.h>
#include <liara/result.h>

#include <cstdint>
#include <new>

struct liara_platform_t
{
    char m_Unused = 0;
};

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

// NOLINTBEGIN(cppcoreguidelines-owning-memory)
// NOLINTBEGIN(readability-identifier-naming)
liara_result_t liara_platform_create(const liara_platform_create_info_t* create_info,
                                     liara_platform_handle_t** out_platform) {
    // NOLINTEND(readability-identifier-naming)
    if (create_info == nullptr) { return LIARA_RESULT_NULL_POINTER; }
    if (create_info->struct_version != LIARA_PLATFORM_CREATE_INFO_VERSION) { return LIARA_RESULT_VERSION_MISMATCH; }

    auto* platform = new (std::nothrow) liara_platform_t {};
    if (platform == nullptr) { return LIARA_RESULT_OUT_OF_MEMORY; }

    *out_platform = platform;
    return LIARA_RESULT_SUCCESS;
}

// NOLINTBEGIN(readability-identifier-naming)
void liara_platform_destroy(liara_platform_handle_t* platform) {
    // NOLINTEND(readability-identifier-naming)
    delete platform;
}  // NOLINTEND(cppcoreguidelines-owning-memory)

// NOLINTBEGIN(readability-identifier-naming)
liara_result_t liara_platform_install_signal_handlers(liara_platform_handle_t* platform) {
    // NOLINTEND(readability-identifier-naming)
    if (platform == nullptr) { return LIARA_RESULT_NULL_POINTER; }
    return Liara::Platform::Shutdown::Install() ? LIARA_RESULT_SUCCESS : LIARA_RESULT_FAILED;
}

// NOLINTBEGIN(readability-identifier-naming)
bool liara_platform_quit_requested(const liara_platform_handle_t* platform) {
    // NOLINTEND(readability-identifier-naming)
    (void)platform;  // Stage 1 has no per-instance quit source. Stage 2's window close button adds one here.
    return Liara::Platform::Shutdown::QuitRequested();
}

// NOLINTBEGIN(readability-identifier-naming)
uint64_t liara_platform_time_now_ns(void) { return Liara::Platform::Time::NowNs(); }

int64_t liara_platform_time_wall_ns(void) { return Liara::Platform::Time::WallNs(); }

void liara_platform_time_sleep_until_ns(const uint64_t deadline_ns) {
    Liara::Platform::Time::SleepUntilNs(deadline_ns);
}

uint64_t liara_platform_time_resolution_ns(void) { return Liara::Platform::Time::ResolutionNs(); }

// NOLINTEND(readability-identifier-naming)
