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

# The window tests run under SDL's offscreen driver, which needs no display. Naming the driver here keeps the knowledge
# that the backend is SDL inside the repository that uses it.
add_executable(test_platform_window_cxx test_platform_window_cxx.cpp)
target_link_libraries(test_platform_window_cxx PRIVATE
        Liara::Interfaces
        Liara::Platform
        doctest::doctest)
add_test(NAME test_platform_window_cxx COMMAND test_platform_window_cxx)
set_tests_properties(test_platform_window_cxx PROPERTIES
        LABELS "unit"
        ENVIRONMENT "SDL_VIDEO_DRIVER=offscreen")

# Needs a real window-system session; CI excludes the `display` label.
add_executable(test_platform_native_window_cxx test_platform_native_window_cxx.cpp)
target_link_libraries(test_platform_native_window_cxx PRIVATE
        Liara::Interfaces
        Liara::Platform
        doctest::doctest)
add_test(NAME test_platform_native_window_cxx COMMAND test_platform_native_window_cxx)
set_tests_properties(test_platform_native_window_cxx PROPERTIES LABELS "display")
