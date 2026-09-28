// NOLINTBEGIN(readability-identifier-naming)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <liara/platform/event.h>
#include <liara/platform/platform.h>
#include <liara/platform/window.h>
#include <liara/result.h>

#include <cstdint>
#include <string_view>

#include <doctest/doctest.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

// Injects events through SDL itself, so it has to share the module's copy of SDL: tests/CMakeLists.txt registers it
// only where that holds, static builds and Windows. Runs under the offscreen driver.

// The helpers below REQUIRE named booleans rather than `a == b`: inside a helper, clang-analyzer reports doctest's
// binary-expression message as a leak, a false positive in doctest.h.

namespace
{
    liara_platform_handle_t* CreatePlatform() {
        constexpr liara_platform_create_info_t info {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION};
        liara_platform_handle_t* platform = nullptr;
        const bool created = liara_platform_create(&info, &platform) == LIARA_RESULT_SUCCESS;
        REQUIRE(created);
        return platform;
    }

    /// Not named CreateWindow: <windows.h>, which doctest includes on Windows, defines that name as a macro.
    liara_platform_window_handle_t* OpenTestWindow(liara_platform_handle_t* platform, const std::string_view title) {
        const liara_platform_window_create_info_t info {
            .struct_version = LIARA_PLATFORM_WINDOW_CREATE_INFO_VERSION,
            .requested_size = {.width = 320, .height = 200},
            .reserved = 0,
            .title = title.data(),
            .title_length = title.size()
        };
        liara_platform_window_handle_t* window = nullptr;
        const bool opened = liara_platform_window_create(platform, &info, &window) == LIARA_RESULT_SUCCESS;
        REQUIRE(opened);
        return window;
    }

    /// The SDL id of the window titled `title`, or 0.
    SDL_WindowID FindSdlWindow(const std::string_view title) {
        int count = 0;
        SDL_Window** windows = SDL_GetWindows(&count);
        SDL_WindowID found = 0;
        for (int i = 0; i < count; ++i) {
            if (std::string_view(SDL_GetWindowTitle(windows[i])) == title) { found = SDL_GetWindowID(windows[i]); }
        }
        SDL_free(static_cast<void*>(windows));
        return found;
    }

    void PushWindowEvent(const SDL_EventType type,
                         const SDL_WindowID id,
                         const Sint32 data1 = 0,
                         const Sint32 data2 = 0) {
        const bool known = id != 0;
        REQUIRE(known);
        SDL_Event event {};
        event.type = type;
        event.window.windowID = id;
        event.window.data1 = data1;
        event.window.data2 = data2;
        REQUIRE(SDL_PushEvent(&event));
    }

    /// Empties whatever creating the windows produced, so a test sees only the events it pushes.
    void DrainAll(liara_platform_handle_t* platform) {
        liara_platform_event_t ignored {};
        while (liara_platform_poll_event(platform, &ignored)) {}
    }
}  // namespace

TEST_CASE("a close request on the last open window is reported and raises the instance's quit flag") {
    liara_platform_handle_t* platform = CreatePlatform();
    liara_platform_window_handle_t* window = OpenTestWindow(platform, "only");
    DrainAll(platform);

    PushWindowEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED, FindSdlWindow("only"));

    liara_platform_event_t event {};
    REQUIRE(liara_platform_poll_event(platform, &event));
    CHECK(event.type == LIARA_PLATFORM_EVENT_WINDOW_CLOSE_REQUESTED);
    CHECK(event.window == window);
    CHECK(liara_platform_quit_requested(platform));

    // A second instance proves the flag raised is this instance's, not the process's.
    liara_platform_handle_t* bystander = CreatePlatform();
    CHECK_FALSE(liara_platform_quit_requested(bystander));
    liara_platform_destroy(bystander);

    liara_platform_window_destroy(window);
    liara_platform_destroy(platform);
}

TEST_CASE("a close request on one window of two is reported and raises nothing") {
    liara_platform_handle_t* platform = CreatePlatform();
    liara_platform_window_handle_t* first = OpenTestWindow(platform, "first");
    liara_platform_window_handle_t* second = OpenTestWindow(platform, "second");
    DrainAll(platform);

    PushWindowEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED, FindSdlWindow("first"));

    liara_platform_event_t event {};
    REQUIRE(liara_platform_poll_event(platform, &event));
    CHECK(event.type == LIARA_PLATFORM_EVENT_WINDOW_CLOSE_REQUESTED);
    CHECK(event.window == first);
    CHECK_FALSE(liara_platform_quit_requested(platform));

    liara_platform_window_destroy(second);
    liara_platform_window_destroy(first);
    liara_platform_destroy(platform);
}

TEST_CASE("once the other window is destroyed, the remaining one is the last") {
    liara_platform_handle_t* platform = CreatePlatform();
    liara_platform_window_handle_t* first = OpenTestWindow(platform, "first");
    liara_platform_window_handle_t* second = OpenTestWindow(platform, "second");
    DrainAll(platform);

    liara_platform_window_destroy(first);
    PushWindowEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED, FindSdlWindow("second"));

    liara_platform_event_t event {};
    REQUIRE(liara_platform_poll_event(platform, &event));
    CHECK(event.window == second);
    CHECK(liara_platform_quit_requested(platform));

    liara_platform_window_destroy(second);
    liara_platform_destroy(platform);
}

TEST_CASE("an instance never receives the events of another instance's window") {
    liara_platform_handle_t* a = CreatePlatform();
    liara_platform_handle_t* b = CreatePlatform();
    liara_platform_window_handle_t* windowA = OpenTestWindow(a, "a");
    liara_platform_window_handle_t* windowB = OpenTestWindow(b, "b");
    DrainAll(a);
    DrainAll(b);

    PushWindowEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED, FindSdlWindow("b"));

    liara_platform_event_t event {};
    CHECK_FALSE(liara_platform_poll_event(a, &event));  // Pumps, and routes b's event to b.
    REQUIRE(liara_platform_poll_event(b, &event));
    CHECK(event.window == windowB);

    liara_platform_window_destroy(windowB);
    liara_platform_window_destroy(windowA);
    liara_platform_destroy(b);
    liara_platform_destroy(a);
}

TEST_CASE("a pixel-size change is reported with the new size") {
    liara_platform_handle_t* platform = CreatePlatform();
    liara_platform_window_handle_t* window = OpenTestWindow(platform, "resized");
    DrainAll(platform);

    PushWindowEvent(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED, FindSdlWindow("resized"), 640, 480);

    liara_platform_event_t event {};
    REQUIRE(liara_platform_poll_event(platform, &event));
    CHECK(event.type == LIARA_PLATFORM_EVENT_WINDOW_RESIZED);
    CHECK(event.window == window);
    CHECK(event.data.resized.pixel_size.width == 640);
    CHECK(event.data.resized.pixel_size.height == 480);

    liara_platform_window_destroy(window);
    liara_platform_destroy(platform);
}

TEST_CASE("a destroyed window's queued events are discarded with it") {
    liara_platform_handle_t* platform = CreatePlatform();
    liara_platform_window_handle_t* first = OpenTestWindow(platform, "first");
    liara_platform_window_handle_t* second = OpenTestWindow(platform, "second");
    DrainAll(platform);

    PushWindowEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED, FindSdlWindow("first"));
    PushWindowEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED, FindSdlWindow("second"));

    liara_platform_event_t event {};
    REQUIRE(liara_platform_poll_event(platform, &event));  // The pump queued both; this returns first's.
    CHECK(event.window == first);

    liara_platform_window_destroy(second);
    CHECK_FALSE(liara_platform_poll_event(platform, &event));  // second's was discarded with it.

    liara_platform_window_destroy(first);
    liara_platform_destroy(platform);
}

TEST_CASE("an event arriving during a drain waits for the next drain") {
    liara_platform_handle_t* platform = CreatePlatform();
    liara_platform_window_handle_t* window = OpenTestWindow(platform, "drain");
    DrainAll(platform);
    const SDL_WindowID id = FindSdlWindow("drain");

    PushWindowEvent(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED, id, 100, 100);
    liara_platform_event_t event {};
    REQUIRE(liara_platform_poll_event(platform, &event));  // Starts a drain, which pumps once.

    PushWindowEvent(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED, id, 200, 200);
    CHECK_FALSE(liara_platform_poll_event(platform, &event));  // Same drain: no second pump, so it ends.

    REQUIRE(liara_platform_poll_event(platform, &event));  // Next drain.
    CHECK(event.data.resized.pixel_size.width == 200);

    liara_platform_window_destroy(window);
    liara_platform_destroy(platform);
}

TEST_CASE("an event's timestamp falls between the push and the poll") {
    liara_platform_handle_t* platform = CreatePlatform();
    liara_platform_window_handle_t* window = OpenTestWindow(platform, "stamped");
    DrainAll(platform);

    const std::uint64_t before = liara_platform_time_now_ns();
    PushWindowEvent(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED, FindSdlWindow("stamped"), 64, 64);
    liara_platform_event_t event {};
    REQUIRE(liara_platform_poll_event(platform, &event));
    const std::uint64_t after = liara_platform_time_now_ns();

    CHECK(event.timestamp_ns >= before);
    CHECK(event.timestamp_ns <= after);

    liara_platform_window_destroy(window);
    liara_platform_destroy(platform);
}

TEST_CASE("the title slice given by pointer and length is the title SDL shows") {
    constexpr std::string_view LONGER = "Sliced title and more";
    liara_platform_handle_t* platform = CreatePlatform();
    liara_platform_window_handle_t* window = OpenTestWindow(platform, LONGER.substr(0, 12));  // "Sliced title"

    CHECK(FindSdlWindow("Sliced title") != 0);

    liara_platform_window_destroy(window);
    liara_platform_destroy(platform);
}

// NOLINTEND(readability-identifier-naming)
