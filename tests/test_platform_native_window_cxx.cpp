// NOLINTBEGIN(readability-identifier-naming)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <liara/native_window.h>
#include <liara/platform/platform.h>
#include <liara/platform/window.h>
#include <liara/result.h>

#include <string_view>

#include <doctest/doctest.h>

// Label `display`: needs a real window-system session, so CI excludes it. It checks the native handle against the
// session the test runs in. Whether the window is visible is not checkable portably, and stays a manual check.

TEST_CASE("liara_platform_window_get_native - describes the session's own window") {
    constexpr liara_platform_create_info_t platformInfo {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION};
    liara_platform_handle_t* platform = nullptr;
    REQUIRE(liara_platform_create(&platformInfo, &platform) == LIARA_RESULT_SUCCESS);

    constexpr std::string_view TITLE = "Liara native window test";
    const liara_platform_window_create_info_t windowInfo {
        .struct_version = LIARA_PLATFORM_WINDOW_CREATE_INFO_VERSION,
        .requested_size = {.width = 320, .height = 200},
        .reserved = 0,
        .title = TITLE.data(),
        .title_length = TITLE.size()
    };
    liara_platform_window_handle_t* window = nullptr;
    REQUIRE(liara_platform_window_create(platform, &windowInfo, &window) == LIARA_RESULT_SUCCESS);

    liara_native_window_t native {};
    REQUIRE(liara_platform_window_get_native(window, &native) == LIARA_RESULT_SUCCESS);

#ifdef _WIN32
    CHECK(native.kind == LIARA_NATIVE_WINDOW_KIND_WIN32);
    CHECK(native.handle.win32.hinstance != nullptr);
    CHECK(native.handle.win32.hwnd != nullptr);
#else
    REQUIRE((native.kind == LIARA_NATIVE_WINDOW_KIND_WAYLAND || native.kind == LIARA_NATIVE_WINDOW_KIND_XLIB));
    if (native.kind == LIARA_NATIVE_WINDOW_KIND_WAYLAND) {
        CHECK(native.handle.wayland.display != nullptr);
        CHECK(native.handle.wayland.surface != nullptr);
    }
    else {
        CHECK(native.handle.xlib.dpy != nullptr);
        CHECK(native.handle.xlib.window != 0);
    }
#endif

    liara_platform_window_destroy(window);
    liara_platform_destroy(platform);
}

// NOLINTEND(readability-identifier-naming)
