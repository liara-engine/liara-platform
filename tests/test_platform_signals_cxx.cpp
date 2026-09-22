// NOLINTBEGIN(readability-identifier-naming)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <liara/platform/platform.h>
#include <liara/result.h>

#include <csignal>

#include <doctest/doctest.h>

// This test installs a process-global signal handler that nothing can uninstall, so it runs in a binary of its own
// rather than beside cases that may want the default disposition for SIGINT.
//
// The Windows half is not covered. SetConsoleCtrlHandler runs its handler on a thread the operating system creates,
// and no portable call provokes it from inside the process, so Windows shutdown is verified by hand until something
// better exists. Saying so here is the point: a test that pretended to cover it would be worse than this comment.

TEST_CASE("liara_platform_quit_requested - goes true after SIGINT and stays true") {
#ifndef _WIN32
    const liara_platform_create_info_t info {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION, .reserved = 0};
    liara_platform_handle_t* platform = nullptr;
    REQUIRE(liara_platform_create(&info, &platform) == LIARA_RESULT_SUCCESS);
    REQUIRE(liara_platform_install_signal_handlers(platform) == LIARA_RESULT_SUCCESS);
    REQUIRE(liara_platform_quit_requested(platform) == false);

    REQUIRE(raise(SIGINT) == 0);

    CHECK(liara_platform_quit_requested(platform) == true);
    CHECK(liara_platform_quit_requested(platform) == true);

    liara_platform_destroy(platform);
#endif
}

// NOLINTEND(readability-identifier-naming)
