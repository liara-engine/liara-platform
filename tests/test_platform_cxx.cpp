// NOLINTBEGIN(readability-identifier-naming)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <liara/abi_version.h>
#include <liara/modules.h>
#include <liara/platform/config.h>
#include <liara/platform/platform.h>
#include <liara/result.h>

#include <string_view>

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
    const liara_platform_create_info_t info {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION, .reserved = 0};
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

    const liara_platform_create_info_t tooOld {.struct_version = 0, .reserved = 0};
    CHECK(liara_platform_create(&tooOld, &platform) == LIARA_RESULT_VERSION_MISMATCH);
    CHECK(platform == nullptr);

    const liara_platform_create_info_t tooNew {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION + 1U,
                                               .reserved = 0};
    CHECK(liara_platform_create(&tooNew, &platform) == LIARA_RESULT_VERSION_MISMATCH);
    CHECK(platform == nullptr);
}

TEST_CASE("liara_platform_destroy - a null handle is a no-op") { liara_platform_destroy(nullptr); }

// NOLINTEND(readability-identifier-naming)
