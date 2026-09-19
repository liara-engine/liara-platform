cmake_minimum_required(VERSION 3.29)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
            "In-source builds are not supported. Build into build/<preset-name>/ instead, which is "
            "what the presets already do:\n"
            "    cmake --preset linux-debug-clang\n"
            "Remove the CMakeCache.txt and CMakeFiles/ this attempt just created before retrying.")
endif()

include(GNUInstallDirs)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.release-please-manifest.json" RELEASE_PLEASE_MANIFEST)
string(JSON PROJECT_VERSION GET ${RELEASE_PLEASE_MANIFEST} ".")
message(STATUS "LiaraPlatform version: ${PROJECT_VERSION}")

project(LiaraPlatform
        VERSION ${PROJECT_VERSION}
        DESCRIPTION "Liara Engine platform abstraction"
        HOMEPAGE_URL "https://github.com/liara-engine/liara-platform"
        LANGUAGES CXX
)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/include/liara/platform/config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/liara/platform/config.h"
        @ONLY)

if(NOT TARGET Liara::Interfaces)
    find_package(LiaraInterfaces CONFIG QUIET)
endif()

if(NOT TARGET Liara::Interfaces)
    message(FATAL_ERROR
            "Liara::Interfaces is not available. Either build liara-platform through the workspace superbuild, "
            "or install liara-interfaces and point find_package(LiaraInterfaces) at it via CMAKE_PREFIX_PATH "
            " (see https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/bootstrap)")
endif()

if(PROJECT_IS_TOP_LEVEL)
    set(LIARA_PLATFORM_IS_TOP_LEVEL ON)
else()
    set(LIARA_PLATFORM_IS_TOP_LEVEL OFF)
endif()

option(LIARA_PLATFORM_BUILD_TESTS
        "Build the liara-platform test suite."
        ${LIARA_PLATFORM_IS_TOP_LEVEL})
option(LIARA_PLATFORM_INSTALL
        "Generate install targets for liara-platform."
        ${LIARA_PLATFORM_IS_TOP_LEVEL})

if(LIARA_PLATFORM_IS_TOP_LEVEL)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

file(GLOB_RECURSE LIARA_PLATFORM_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(liara_platform ${LIARA_PLATFORM_SOURCES})
add_library(Liara::Platform ALIAS liara_platform)

liara_configure_target(liara_platform)

target_link_libraries(liara_platform PUBLIC Liara::Interfaces)

target_include_directories(liara_platform PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(liara_platform PRIVATE LIARA_PLATFORM_BUILD)

set_target_properties(liara_platform PROPERTIES
        EXPORT_NAME Platform
        SOVERSION ${PROJECT_VERSION_MAJOR}
        VERSION ${PROJECT_VERSION}
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(liara_platform PUBLIC LIARA_PLATFORM_SHARED)
else()
    target_compile_definitions(liara_platform PUBLIC LIARA_PLATFORM_STATIC)
endif()

if(LIARA_PLATFORM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(LIARA_PLATFORM_INSTALL)
    include(CMakePackageConfigHelpers)

    install(TARGETS liara_platform
            EXPORT LiaraPlatformTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT liara-platform-runtime
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT liara-platform-runtime
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT liara-platform-dev
    )

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/liara/platform"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/liara"
            COMPONENT liara-platform-dev
            FILES_MATCHING PATTERN "*.h"
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/liara/platform/config.h"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/liara/platform"
            COMPONENT liara-platform-dev
    )

    install(EXPORT LiaraPlatformTargets
            FILE LiaraPlatformTargets.cmake
            NAMESPACE Liara::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LiaraPlatform
            COMPONENT liara-platform-dev
    )

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/LiaraPlatformConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/LiaraPlatformConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/LiaraPlatformConfig.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LiaraPlatform
    )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/LiaraPlatformConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/LiaraPlatformConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LiaraPlatform
            COMPONENT liara-platform-dev
    )
endif()
