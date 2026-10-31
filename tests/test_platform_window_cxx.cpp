// NOLINTBEGIN(readability-identifier-naming)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <liara/extent.h>
#include <liara/native_window.h>
#include <liara/platform/event.h>
#include <liara/platform/platform.h>
#include <liara/platform/window.h>
#include <liara/result.h>

#include <cstdint>
#include <string_view>

#ifdef __linux__
    #include <sys/resource.h>
#endif

#include <doctest/doctest.h>

// Runs under SDL's offscreen video driver (SDL_VIDEO_DRIVER=offscreen, set by tests/CMakeLists.txt), so it needs no
// display and runs in CI. The one test that needs a real window system is test_platform_native_window_cxx.

namespace
{
    constexpr std::string_view TITLE = "Liara window test";

    liara_platform_window_create_info_t ValidWindowInfo() {
        return {
            .struct_version = LIARA_PLATFORM_WINDOW_CREATE_INFO_VERSION,
            .requested_size = {.width = 320, .height = 200},
            .reserved = 0,
            .title = TITLE.data(),
            .title_length = TITLE.size()
        };
    }

    struct PlatformFixture
    {
        liara_platform_handle_t* platform = nullptr;

        PlatformFixture() {
            constexpr liara_platform_create_info_t info {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION};
            REQUIRE(liara_platform_create(&info, &platform) == LIARA_RESULT_SUCCESS);
        }

        ~PlatformFixture() { liara_platform_destroy(platform); }

        PlatformFixture(const PlatformFixture&) = delete;
        PlatformFixture& operator=(const PlatformFixture&) = delete;
        PlatformFixture(PlatformFixture&&) = delete;
        PlatformFixture& operator=(PlatformFixture&&) = delete;

        [[nodiscard]] liara_platform_window_handle_t* OpenWindow() const {
            const liara_platform_window_create_info_t info = ValidWindowInfo();
            liara_platform_window_handle_t* window = nullptr;
            // Named booleans rather than REQUIRE(a == b): inside a helper, clang-analyzer reports doctest's
            // binary-expression message as a leak, a false positive in doctest.h.
            const bool opened =
                liara_platform_window_create(platform, &info, &window) == LIARA_RESULT_SUCCESS && window != nullptr;
            REQUIRE(opened);
            return window;
        }
    };
}  // namespace

TEST_CASE_FIXTURE(PlatformFixture, "liara_platform_window_create - yields a window and destroys it") {
    liara_platform_window_handle_t* window = OpenWindow();
    liara_platform_window_destroy(window);
}

TEST_CASE_FIXTURE(PlatformFixture, "liara_platform_window_create - opens again after every window was destroyed") {
    // The first destroy takes SDL's video reference count back to zero; the second create has to start it again.
    liara_platform_window_destroy(OpenWindow());
    liara_platform_window_destroy(OpenWindow());
}

TEST_CASE_FIXTURE(PlatformFixture, "liara_platform_window_create - refuses null pointers") {
    const liara_platform_window_create_info_t info = ValidWindowInfo();
    liara_platform_window_handle_t* window = nullptr;

    CHECK(liara_platform_window_create(nullptr, &info, &window) == LIARA_RESULT_NULL_POINTER);
    CHECK(liara_platform_window_create(platform, nullptr, &window) == LIARA_RESULT_NULL_POINTER);

    liara_platform_window_create_info_t nullTitle = ValidWindowInfo();
    nullTitle.title = nullptr;
    CHECK(liara_platform_window_create(platform, &nullTitle, &window) == LIARA_RESULT_NULL_POINTER);

    CHECK(window == nullptr);
}

TEST_CASE_FIXTURE(PlatformFixture, "liara_platform_window_create - takes an empty title") {
    liara_platform_window_create_info_t info = ValidWindowInfo();
    info.title = nullptr;
    info.title_length = 0;
    liara_platform_window_handle_t* window = nullptr;

    REQUIRE(liara_platform_window_create(platform, &info, &window) == LIARA_RESULT_SUCCESS);
    liara_platform_window_destroy(window);
}

TEST_CASE_FIXTURE(PlatformFixture, "liara_platform_window_create - takes a title that is not null-terminated") {
    constexpr std::string_view LONGER = "Liara and more";
    liara_platform_window_create_info_t info = ValidWindowInfo();
    info.title = LONGER.data();
    info.title_length = 5;  // "Liara", followed by more characters rather than a terminator.
    liara_platform_window_handle_t* window = nullptr;

    REQUIRE(liara_platform_window_create(platform, &info, &window) == LIARA_RESULT_SUCCESS);
    liara_platform_window_destroy(window);
}

TEST_CASE_FIXTURE(PlatformFixture, "liara_platform_window_create - refuses a struct version it does not understand") {
    liara_platform_window_handle_t* window = nullptr;

    liara_platform_window_create_info_t tooOld = ValidWindowInfo();
    tooOld.struct_version = 0;
    CHECK(liara_platform_window_create(platform, &tooOld, &window) == LIARA_RESULT_VERSION_MISMATCH);

    liara_platform_window_create_info_t tooNew = ValidWindowInfo();
    tooNew.struct_version = LIARA_PLATFORM_WINDOW_CREATE_INFO_VERSION + 1U;
    CHECK(liara_platform_window_create(platform, &tooNew, &window) == LIARA_RESULT_VERSION_MISMATCH);

    CHECK(window == nullptr);
}

TEST_CASE_FIXTURE(PlatformFixture, "liara_platform_window_create - refuses a zero size") {
    liara_platform_window_handle_t* window = nullptr;

    liara_platform_window_create_info_t noWidth = ValidWindowInfo();
    noWidth.requested_size.width = 0;
    CHECK(liara_platform_window_create(platform, &noWidth, &window) == LIARA_RESULT_INVALID_ARGUMENT);

    liara_platform_window_create_info_t noHeight = ValidWindowInfo();
    noHeight.requested_size.height = 0;
    CHECK(liara_platform_window_create(platform, &noHeight, &window) == LIARA_RESULT_INVALID_ARGUMENT);

    CHECK(window == nullptr);
}

TEST_CASE_FIXTURE(PlatformFixture,
                  "liara_platform_window_create - survives a size beyond what the window system takes") {
    liara_platform_window_create_info_t info = ValidWindowInfo();
    info.requested_size = {.width = UINT32_MAX, .height = UINT32_MAX};
    liara_platform_window_handle_t* window = nullptr;

    const liara_result_t result = liara_platform_window_create(platform, &info, &window);

    // SDL may refuse the window or clamp it, and either is acceptable. A crash, or a wrapped negative size reaching
    // SDL, is not.
    CHECK((result == LIARA_RESULT_SUCCESS || result == LIARA_RESULT_FAILED));
    if (result == LIARA_RESULT_SUCCESS) { liara_platform_window_destroy(window); }
    else { CHECK(window == nullptr); }

#ifdef __linux__
    // Peak resident memory of the whole binary so far, in kilobytes. The placeholder must not paint a surface of the
    // clamped size (16384 x 16384 pixels, a GiB), which every CI run would otherwise pay for.
    // glibc defines rusage in a bits/ header that include-cleaner does not map back to <sys/resource.h>.
    rusage usage {};  // NOLINT(misc-include-cleaner)
    // Named booleans rather than CHECK(a < b), which clang-analyzer reports as a leak in doctest.h, a false positive.
    const bool measured = getrusage(RUSAGE_SELF, &usage) == 0;
    REQUIRE(measured);
    const bool withinBudget = usage.ru_maxrss < 512L * 1024L;
    CHECK(withinBudget);
#endif
}

TEST_CASE("liara_platform_window_destroy - a null handle is a no-op") { liara_platform_window_destroy(nullptr); }

TEST_CASE_FIXTURE(PlatformFixture, "liara_platform_window_get_pixel_size - reports a non-zero size") {
    liara_platform_window_handle_t* window = OpenWindow();
    liara_extent2d_t size {};

    REQUIRE(liara_platform_window_get_pixel_size(window, &size) == LIARA_RESULT_SUCCESS);
    CHECK(size.width > 0);
    CHECK(size.height > 0);

    liara_platform_window_destroy(window);
}

TEST_CASE("liara_platform_window_get_pixel_size - refuses a null window") {
    liara_extent2d_t size {};
    CHECK(liara_platform_window_get_pixel_size(nullptr, &size) == LIARA_RESULT_NULL_POINTER);
}

TEST_CASE_FIXTURE(PlatformFixture, "liara_platform_window_get_native - reports no native window under offscreen") {
    liara_platform_window_handle_t* window = OpenWindow();
    liara_native_window_t native {};
    native.kind = LIARA_NATIVE_WINDOW_KIND_WAYLAND;  // Must be overwritten.

    CHECK(liara_platform_window_get_native(window, &native) == LIARA_RESULT_NOT_SUPPORTED);
    CHECK(native.kind == LIARA_NATIVE_WINDOW_KIND_NONE);

    liara_platform_window_destroy(window);
}

TEST_CASE("liara_platform_window_get_native - refuses a null window") {
    liara_native_window_t native {};
    CHECK(liara_platform_window_get_native(nullptr, &native) == LIARA_RESULT_NULL_POINTER);
}

TEST_CASE_FIXTURE(PlatformFixture, "liara_platform_quit_requested - stays false once a window exists") {
    liara_platform_window_handle_t* window = OpenWindow();
    CHECK(liara_platform_quit_requested(platform) == false);
    liara_platform_window_destroy(window);
}

TEST_CASE_FIXTURE(PlatformFixture, "liara_platform_poll_event - is false on an instance that never opened a window") {
    // No window means SDL's video subsystem never started; the poll must not call into an uninitialised SDL.
    liara_platform_event_t event {};
    CHECK(liara_platform_poll_event(platform, &event) == false);
    CHECK(event.type == LIARA_PLATFORM_EVENT_NONE);
}

TEST_CASE_FIXTURE(PlatformFixture, "liara_platform_poll_event - is false when nothing happened to a window") {
    liara_platform_window_handle_t* window = OpenWindow();
    liara_platform_event_t event {};

    // Drain whatever creating the window produced (the offscreen driver may report its size), then expect silence.
    while (liara_platform_poll_event(platform, &event)) {}
    CHECK(liara_platform_poll_event(platform, &event) == false);

    liara_platform_window_destroy(window);
}

// NOLINTEND(readability-identifier-naming)
