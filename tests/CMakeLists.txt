find_package(doctest CONFIG REQUIRED)

add_executable(test_platform_functions_cxx test_platform_cxx.cpp)
target_link_libraries(test_platform_functions_cxx PRIVATE
        Liara::Interfaces
        Liara::Platform
        doctest::doctest)
add_test(NAME test_platform_functions_cxx COMMAND test_platform_functions_cxx)
set_tests_properties(test_platform_functions_cxx PROPERTIES LABELS "unit")
