find_package(doctest CONFIG REQUIRED)

add_executable(test_platform_functions_cxx test_platform_cxx.cpp)
target_link_libraries(test_platform_functions_cxx PRIVATE
        Liara::Interfaces
        Liara::Platform
        doctest::doctest)
add_test(NAME test_platform_functions_cxx COMMAND test_platform_functions_cxx)
set_tests_properties(test_platform_functions_cxx PROPERTIES LABELS "unit")

add_executable(test_platform_signals_cxx test_platform_signals_cxx.cpp)
target_link_libraries(test_platform_signals_cxx PRIVATE
        Liara::Interfaces
        Liara::Platform
        doctest::doctest)
add_test(NAME test_platform_signals_cxx COMMAND test_platform_signals_cxx)
set_tests_properties(test_platform_signals_cxx PROPERTIES LABELS "unit")

# The window tests run under SDL's offscreen driver, which needs no display, and with SDL's window surface kept a
# software framebuffer, since SDL otherwise backs it with a GPU renderer and the tests would depend on the machine's GL
# driver. Naming both here keeps the knowledge that the backend is SDL inside the repository that uses it.
add_executable(test_platform_window_cxx test_platform_window_cxx.cpp)
target_link_libraries(test_platform_window_cxx PRIVATE
        Liara::Interfaces
        Liara::Platform
        doctest::doctest)
add_test(NAME test_platform_window_cxx COMMAND test_platform_window_cxx)
set_tests_properties(test_platform_window_cxx PROPERTIES
        LABELS "unit"
        ENVIRONMENT "SDL_VIDEO_DRIVER=offscreen;SDL_FRAMEBUFFER_ACCELERATION=0")

# Needs a real window-system session; CI excludes the `display` label.
add_executable(test_platform_native_window_cxx test_platform_native_window_cxx.cpp)
target_link_libraries(test_platform_native_window_cxx PRIVATE
        Liara::Interfaces
        Liara::Platform
        doctest::doctest)
add_test(NAME test_platform_native_window_cxx COMMAND test_platform_native_window_cxx)
set_tests_properties(test_platform_native_window_cxx PROPERTIES LABELS "display")

# These inject events through SDL, so they must share the module's copy of it. That holds for static builds, and on
# Windows where SDL3.dll is the only copy. Under the Linux shared presets SDL sits inside liara_platform.so, hidden by
# --exclude-libs, and the test would link a second copy. What they check, translation and routing, does not depend on
# the link mode.
if(NOT BUILD_SHARED_LIBS OR WIN32)
    find_package(SDL3 CONFIG REQUIRED)
    foreach(test IN ITEMS test_platform_events_cxx test_platform_sdl_quit_cxx)
        add_executable(${test} ${test}.cpp)
        target_link_libraries(${test} PRIVATE
                Liara::Interfaces
                Liara::Platform
                SDL3::SDL3
                doctest::doctest)
        add_test(NAME ${test} COMMAND ${test})
        set_tests_properties(${test} PROPERTIES
                LABELS "unit"
                ENVIRONMENT "SDL_VIDEO_DRIVER=offscreen;SDL_FRAMEBUFFER_ACCELERATION=0")
    endforeach()
endif()
