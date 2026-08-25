cmake_minimum_required(VERSION 3.20)
project(AngelCausalBoundarySDK VERSION 1.1.0 LANGUAGES CXX)

option(ANGEL_BUILD_TESTS "Build the executable verification suite" ON)
option(ANGEL_ENABLE_SANITIZERS "Enable address and undefined-behavior checks" OFF)

add_library(angel_causal_boundary STATIC
    src/boundary_pipeline.cpp
    src/cyclic_structure.cpp
    src/diagnostics.cpp
    src/encoding.cpp
    src/native_factorial.cpp
    src/prime_pipeline.cpp
    src/session.cpp
    src/state.cpp)

target_compile_features(angel_causal_boundary PUBLIC cxx_std_20)
target_include_directories(angel_causal_boundary
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/sealed_core/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_options(angel_causal_boundary PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)

if(ANGEL_ENABLE_SANITIZERS)
    target_compile_options(angel_causal_boundary PRIVATE
        -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(angel_causal_boundary PRIVATE
        -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

if(ANGEL_BUILD_TESTS)
    enable_testing()

    function(add_angel_test target source)
        add_executable(${target} ${source})
        target_link_libraries(${target} PRIVATE angel_causal_boundary)
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
        if(ANGEL_ENABLE_SANITIZERS)
            target_compile_options(${target} PRIVATE
                -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE
                -fsanitize=address,undefined -fno-omit-frame-pointer)
        endif()
        add_test(NAME ${target} COMMAND ${target})
    endfunction()

    add_angel_test(boundary_pipeline_tests tests/boundary_pipeline_tests.cpp)
    add_angel_test(cyclic_structure_tests tests/cyclic_structure_tests.cpp)
    add_angel_test(native_factorial_tests tests/native_factorial_tests.cpp)
    add_angel_test(prime_pipeline_tests tests/prime_pipeline_tests.cpp)
    add_angel_test(behavior_tests tests/behavior_tests.cpp)

    add_executable(complexity_probe tests/complexity_probe.cpp)
    target_link_libraries(complexity_probe PRIVATE angel_causal_boundary)

    add_executable(boundary_pipeline_example examples/boundary_pipeline.cpp)
    target_link_libraries(boundary_pipeline_example PRIVATE angel_causal_boundary)

    add_executable(prime_pipeline_example examples/prime_pipeline.cpp)
    target_link_libraries(prime_pipeline_example PRIVATE angel_causal_boundary)

    add_executable(cyclic_structure_example examples/cyclic_structure_pipeline.cpp)
    target_link_libraries(cyclic_structure_example PRIVATE angel_causal_boundary)

    add_executable(native_factorial_example examples/native_factorial_pipeline.cpp)
    target_link_libraries(native_factorial_example PRIVATE angel_causal_boundary)
endif()
