// NOLINTBEGIN(readability-identifier-naming)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <liara/abi_version.h>
#include <liara/modules.h>
#include <liara/platform/config.h>
#include <liara/platform/platform.h>

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

// NOLINTEND(readability-identifier-naming)
