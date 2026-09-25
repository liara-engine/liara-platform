// NOLINTBEGIN(readability-identifier-naming)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <liara/abi_version.h>
#include <liara/modules.h>
#include <liara/platform/config.h>
#include <liara/platform/platform.h>
#include <liara/result.h>

#include <chrono>
#include <cstdint>
#include <string_view>
#include <thread>

#include <doctest/doctest.h>

TEST_CASE("liara_platform_info - reports a well-formed module info") {
    const liara_module_info_t* info = liara_platform_info();

    REQUIRE(info != nullptr);
    CHECK(info->struct_version == LIARA_MODULE_INFO_VERSION);
    CHECK(std::string_view(info->module_name) == "Platform");
    CHECK(std::string_view(info->module_version_str) == LIARA_PLATFORM_VERSION_STRING);
    CHECK(info->module_version == LIARA_PLATFORM_VERSION);
}

TEST_CASE("liara_platform_info - is the same object on every call") {
    CHECK(liara_platform_info() == liara_platform_info());
}

TEST_CASE("liara_platform_abi_version - agrees with the contract it was built against") {
    CHECK(liara_platform_abi_version() == LIARA_ABI_VERSION);
    CHECK(std::string_view(liara_platform_info()->abi_version_str) == LIARA_ABI_VERSION_STR);
}

TEST_CASE("liara_platform_abi_version - is accepted by the negotiation rule") {
    const liara_version_compat_t compat = liara_abi_is_compatible(liara_platform_abi_version());
    CHECK((compat == LIARA_VERSION_COMPAT_EXACT || compat == LIARA_VERSION_COMPAT_COMPATIBLE));
}

TEST_CASE("liara_platform_create - yields a handle and destroys it") {
    constexpr liara_platform_create_info_t info {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION};
    liara_platform_handle_t* platform = nullptr;

    REQUIRE(liara_platform_create(&info, &platform) == LIARA_RESULT_SUCCESS);
    REQUIRE(platform != nullptr);

    // A void destroy asserts by completing: the call compiles, links, runs and frees.
    liara_platform_destroy(platform);
}

TEST_CASE("liara_platform_create - refuses a null create info") {
    liara_platform_handle_t* platform = nullptr;

    CHECK(liara_platform_create(nullptr, &platform) == LIARA_RESULT_NULL_POINTER);
    CHECK(platform == nullptr);
}

TEST_CASE("liara_platform_create - refuses a struct version it does not understand") {
    liara_platform_handle_t* platform = nullptr;

    constexpr liara_platform_create_info_t tooOld {.struct_version = 0};
    CHECK(liara_platform_create(&tooOld, &platform) == LIARA_RESULT_VERSION_MISMATCH);
    CHECK(platform == nullptr);

    constexpr liara_platform_create_info_t tooNew {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION + 1U};
    CHECK(liara_platform_create(&tooNew, &platform) == LIARA_RESULT_VERSION_MISMATCH);
    CHECK(platform == nullptr);
}

TEST_CASE("liara_platform_destroy - a null handle is a no-op") { liara_platform_destroy(nullptr); }

TEST_CASE("liara_platform_quit_requested - is false on a fresh handle") {
    constexpr liara_platform_create_info_t info {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION};
    liara_platform_handle_t* platform = nullptr;
    REQUIRE(liara_platform_create(&info, &platform) == LIARA_RESULT_SUCCESS);

    CHECK(liara_platform_quit_requested(platform) == false);

    liara_platform_destroy(platform);
}

TEST_CASE("liara_platform_install_signal_handlers - succeeds, and again for a second handle") {
    constexpr liara_platform_create_info_t info {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION};
    liara_platform_handle_t* first = nullptr;
    liara_platform_handle_t* second = nullptr;
    REQUIRE(liara_platform_create(&info, &first) == LIARA_RESULT_SUCCESS);
    REQUIRE(liara_platform_create(&info, &second) == LIARA_RESULT_SUCCESS);

    CHECK(liara_platform_install_signal_handlers(first) == LIARA_RESULT_SUCCESS);
    CHECK(liara_platform_install_signal_handlers(second) == LIARA_RESULT_SUCCESS);

    liara_platform_destroy(second);
    liara_platform_destroy(first);
}

TEST_CASE("liara_platform_install_signal_handlers - refuses a null handle") {
    CHECK(liara_platform_install_signal_handlers(nullptr) == LIARA_RESULT_NULL_POINTER);
}

TEST_CASE("liara_platform_time_now_ns - never decreases") {
    constexpr int READINGS = 1000;

    uint64_t previous = liara_platform_time_now_ns();
    for (int i = 0; i < READINGS; ++i) {
        const uint64_t current = liara_platform_time_now_ns();
        REQUIRE(current >= previous);
        previous = current;
    }
}

TEST_CASE("liara_platform_time_now_ns - agrees with steady_clock") {
    constexpr auto SAMPLE_DURATION = std::chrono::milliseconds(10);
    constexpr auto MAX_CLOCK_DELTA = std::chrono::nanoseconds(1000);

    const auto std_start = std::chrono::steady_clock::now();
    const uint64_t liara_start = liara_platform_time_now_ns();
    std::this_thread::sleep_for(SAMPLE_DURATION);
    const uint64_t liara_end = liara_platform_time_now_ns();
    const auto std_end = std::chrono::steady_clock::now();

    const auto std_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std_end - std_start);
    const auto liara_elapsed = std::chrono::nanoseconds(liara_end - liara_start);
    const auto delta = std_elapsed > liara_elapsed ? std_elapsed - liara_elapsed : liara_elapsed - std_elapsed;

    CHECK(std_elapsed >= SAMPLE_DURATION);
    CHECK(delta <= MAX_CLOCK_DELTA);
}

TEST_CASE("liara_platform_time_sleep_until_ns - reaches the deadline") {
    constexpr uint64_t TEN_MS_NS = 10ULL * 1000ULL * 1000ULL;
    const uint64_t deadline = liara_platform_time_now_ns() + TEN_MS_NS;

    liara_platform_time_sleep_until_ns(deadline);

    // Only the "at least" direction is asserted. The contract promises no upper bound on overshoot, a loaded machine
    // makes one unpredictable, and asserting one here would be a flake generator. Do not "fix" this by adding a
    // ceiling.
    CHECK(liara_platform_time_now_ns() >= deadline);
}

TEST_CASE("liara_platform_time_sleep_until_ns - a past deadline returns") {
    const uint64_t now = liara_platform_time_now_ns();

    liara_platform_time_sleep_until_ns(now);
    liara_platform_time_sleep_until_ns(now > 0U ? now - 1U : 0U);
}

TEST_CASE("liara_platform_time_wall_ns - lands in a plausible band") {
    constexpr int64_t YEAR_2020_NS = 1577836800LL * 1000LL * 1000LL * 1000LL;  // 2020-01-01T00:00:00Z
    constexpr int64_t YEAR_2100_NS = 4102444800LL * 1000LL * 1000LL * 1000LL;  // 2100-01-01T00:00:00Z

    const int64_t wall = liara_platform_time_wall_ns();

    // Catches a unit mistake (microseconds taken for nanoseconds) and an epoch mistake, and cannot flake.
    CHECK(wall > YEAR_2020_NS);
    CHECK(wall < YEAR_2100_NS);
}

TEST_CASE("liara_platform_time_wall_ns - agrees with system_clock") {
    constexpr auto MAX_CLOCK_DELTA = std::chrono::nanoseconds(1000);

    const auto std_before = std::chrono::system_clock::now();
    const int64_t liara_wall = liara_platform_time_wall_ns();
    const auto std_after = std::chrono::system_clock::now();

    const auto std_before_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std_before.time_since_epoch()).count();
    const auto std_after_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std_after.time_since_epoch()).count();
    const auto delta_before = std::chrono::nanoseconds(liara_wall - std_before_ns);
    const auto delta_after = std::chrono::nanoseconds(liara_wall - std_after_ns);

    CHECK(delta_before <= MAX_CLOCK_DELTA);
    CHECK(delta_after >= -MAX_CLOCK_DELTA);
}

TEST_CASE("liara_platform_time_resolution_ns - reports a positive resolution") {
    const uint64_t resolution = liara_platform_time_resolution_ns();

    CHECK(resolution > 0U);
    CHECK(resolution <=
          static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::steady_clock::duration(1)).count()));
}

// NOLINTEND(readability-identifier-naming)
