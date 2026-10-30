#include "Instance.h"
#include "Shutdown.h"
#include "Time.h"
#include "Window.h"

#include <liara/abi_version.h>
#include <liara/extent.h>
#include <liara/modules.h>
#include <liara/native_window.h>
#include <liara/platform/config.h>
#include <liara/platform/event.h>
#include <liara/platform/platform.h>
#include <liara/platform/window.h>
#include <liara/result.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <new>
#include <string_view>

struct liara_platform_t
{ Liara::Platform::Instance m_Instance; };

struct liara_platform_window_t
{ std::unique_ptr<Liara::Platform::Window> m_Window; };

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

    try {
        auto platform = std::make_unique<liara_platform_t>();
        *out_platform = platform.release();
        return LIARA_RESULT_SUCCESS;
    }
    catch (const std::bad_alloc&) {
        return LIARA_RESULT_OUT_OF_MEMORY;
    }
}

// NOLINTBEGIN(readability-identifier-naming, misc-const-correctness)
void liara_platform_destroy(liara_platform_handle_t* platform) {
    // NOLINTEND(readability-identifier-naming, misc-const-correctness)
    if (platform == nullptr) { return; }
#ifdef LIARA_ENABLE_ASSERTS
    // Windows are children of their instance and go first; one still open here is a programming error, caught in debug
    // builds. Not assert(): against glibc 2.44, clang-tidy's misc-static-assert flags every assert() as constant.
    if (platform->m_Instance.OpenWindowCount() != 0) {
        (void)std::fputs("liara_platform_destroy: the instance still has open windows; destroy them first.\n", stderr);
        std::abort();
    }
#endif
    delete platform;
}

// NOLINTBEGIN(readability-identifier-naming)
liara_result_t liara_platform_window_create(liara_platform_handle_t* platform,
                                            const liara_platform_window_create_info_t* create_info,
                                            liara_platform_window_handle_t** out_window) {
    // NOLINTEND(readability-identifier-naming)
    if (platform == nullptr || create_info == nullptr) { return LIARA_RESULT_NULL_POINTER; }
    if (create_info->struct_version != LIARA_PLATFORM_WINDOW_CREATE_INFO_VERSION) {
        return LIARA_RESULT_VERSION_MISMATCH;
    }
    if (create_info->title == nullptr && create_info->title_length != 0) { return LIARA_RESULT_NULL_POINTER; }
    if (create_info->requested_size.width == 0 || create_info->requested_size.height == 0) {
        return LIARA_RESULT_INVALID_ARGUMENT;
    }

    const std::string_view title = create_info->title == nullptr
                                       ? std::string_view {}
                                       : std::string_view(create_info->title, create_info->title_length);

    try {
        auto window = std::make_unique<liara_platform_window_t>();
        window->m_Window =
            Liara::Platform::Window::Create(platform->m_Instance, window.get(), create_info->requested_size, title);
        if (window->m_Window == nullptr) { return LIARA_RESULT_FAILED; }
        *out_window = window.release();
        return LIARA_RESULT_SUCCESS;
    }
    catch (const std::bad_alloc&) {
        return LIARA_RESULT_OUT_OF_MEMORY;
    }
    catch (...) {
        return LIARA_RESULT_FAILED;  // Nothing unwinds through the boundary.
    }
}

// NOLINTBEGIN(readability-identifier-naming, misc-const-correctness)
void liara_platform_window_destroy(liara_platform_window_handle_t* window) {
    // NOLINTEND(readability-identifier-naming, misc-const-correctness)
    delete window;
}  // NOLINTEND(cppcoreguidelines-owning-memory)

// NOLINTBEGIN(readability-identifier-naming, misc-const-correctness)
liara_result_t liara_platform_install_signal_handlers(liara_platform_handle_t* platform) {
    // NOLINTEND(readability-identifier-naming, misc-const-correctness)
    if (platform == nullptr) { return LIARA_RESULT_NULL_POINTER; }
    return Liara::Platform::Shutdown::Install() ? LIARA_RESULT_SUCCESS : LIARA_RESULT_FAILED;
}

// NOLINTBEGIN(readability-identifier-naming)
bool liara_platform_quit_requested(const liara_platform_handle_t* platform) {
    // NOLINTEND(readability-identifier-naming)
    return Liara::Platform::Shutdown::QuitRequested() || platform->m_Instance.QuitRequested();
}

// NOLINTBEGIN(readability-identifier-naming)
bool liara_platform_poll_event(liara_platform_handle_t* platform, liara_platform_event_t* out_event) {
    // NOLINTEND(readability-identifier-naming)
    try {
        return platform->m_Instance.Poll(*out_event);
    }
    catch (...) {
        return false;  // Nothing unwinds through the boundary; an event lost to an allocation failure is dropped.
    }
}

// NOLINTBEGIN(readability-identifier-naming)
uint64_t liara_platform_time_now_ns(void) { return Liara::Platform::Time::NowNs(); }

int64_t liara_platform_time_wall_ns(void) { return Liara::Platform::Time::WallNs(); }

void liara_platform_time_sleep_until_ns(const uint64_t deadline_ns) {
    Liara::Platform::Time::SleepUntilNs(deadline_ns);
}

uint64_t liara_platform_time_resolution_ns(void) { return Liara::Platform::Time::ResolutionNs(); }

liara_result_t liara_platform_window_get_pixel_size(const liara_platform_window_handle_t* window,
                                                    liara_extent2d_t* out_pixel_size) {
    if (window == nullptr) { return LIARA_RESULT_NULL_POINTER; }
    return window->m_Window->PixelSize(*out_pixel_size) ? LIARA_RESULT_SUCCESS : LIARA_RESULT_FAILED;
}

liara_result_t liara_platform_window_get_native(const liara_platform_window_handle_t* window,
                                                liara_native_window_t* out_native) {
    if (window == nullptr) { return LIARA_RESULT_NULL_POINTER; }
    return window->m_Window->Native(*out_native) ? LIARA_RESULT_SUCCESS : LIARA_RESULT_NOT_SUPPORTED;
}

// NOLINTEND(readability-identifier-naming)
