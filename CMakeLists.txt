cmake_minimum_required(VERSION 3.12)
project(test-fifocopy)

enable_testing()

add_compile_options("-Wall")
add_compile_options("-Werror")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(TARGET_NAME ${CMAKE_PROJECT_NAME})
set(TARGET_SRC
    main.cpp
    testFifoCopy.cpp
)
add_executable(${TARGET_NAME} ${TARGET_SRC})
set_target_properties(${TARGET_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED TRUE
    CXX_EXTENSIONS OFF
)
target_include_directories(${TARGET_NAME} PRIVATE
)
target_compile_definitions(${TARGET_NAME} PRIVATE
)
target_link_libraries(${TARGET_NAME}
    gtest
    gmock
)

add_test(NAME ${CMAKE_PROJECT_NAME} COMMAND ${TARGET_NAME})
