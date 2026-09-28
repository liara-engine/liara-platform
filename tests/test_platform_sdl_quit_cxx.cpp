// NOLINTBEGIN(readability-identifier-naming)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <liara/platform/event.h>
#include <liara/platform/platform.h>
#include <liara/platform/window.h>
#include <liara/result.h>

#include <string_view>

#include <doctest/doctest.h>
#include <SDL3/SDL_events.h>

// Raises the process-global quit flag, which is sticky, so it runs in a binary of its own like the signal test.
// Shares the module's copy of SDL, so it is registered where test_platform_events_cxx is.

TEST_CASE("an SDL_EVENT_QUIT raises the process-global flag") {
    constexpr liara_platform_create_info_t info {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION};
    liara_platform_handle_t* withWindow = nullptr;
    liara_platform_handle_t* headless = nullptr;
    REQUIRE(liara_platform_create(&info, &withWindow) == LIARA_RESULT_SUCCESS);
    REQUIRE(liara_platform_create(&info, &headless) == LIARA_RESULT_SUCCESS);

    // The pump runs only while SDL's video subsystem is up, so one instance holds a window.
    constexpr std::string_view TITLE = "quit";
    const liara_platform_window_create_info_t windowInfo {
        .struct_version = LIARA_PLATFORM_WINDOW_CREATE_INFO_VERSION,
        .requested_size = {.width = 320, .height = 200},
        .reserved = 0,
        .title = TITLE.data(),
        .title_length = TITLE.size()
    };
    liara_platform_window_handle_t* window = nullptr;
    REQUIRE(liara_platform_window_create(withWindow, &windowInfo, &window) == LIARA_RESULT_SUCCESS);
    REQUIRE_FALSE(liara_platform_quit_requested(headless));

    SDL_Event quit {};
    quit.type = SDL_EVENT_QUIT;
    REQUIRE(SDL_PushEvent(&quit));

    liara_platform_event_t event {};
    while (liara_platform_poll_event(withWindow, &event)) {
        CHECK(event.type != LIARA_PLATFORM_EVENT_NONE);  // A quit is a flag, never an event.
    }

    // The headless instance has no window, so only the process-global flag can make this true.
    CHECK(liara_platform_quit_requested(headless));
    CHECK(liara_platform_quit_requested(withWindow));

    liara_platform_window_destroy(window);
    liara_platform_destroy(headless);
    liara_platform_destroy(withWindow);
}

// NOLINTEND(readability-identifier-naming)
