cmake_minimum_required(VERSION 3.30)
project(AetherTests LANGUAGES C CXX)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" IS_TOP_LEVEL)

if(IS_TOP_LEVEL)
    enable_testing()
endif()

################################################################################
# GoogleTest
################################################################################
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/6910c9d9165801d8827d628cb72eb7ea9dd538c5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

################################################################################
# Aether library
################################################################################
if(IS_TOP_LEVEL)
    find_package(Aether REQUIRED)
endif()

################################################################################
# Aether tests
################################################################################
set(${PROJECT_NAME}_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/aether_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/buffer_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cobs_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/err_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hashmap_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/leb128_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/random_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/router_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/serial_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/socket_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tick_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/transport_tests.cc
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_SRCS})
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        ${${PROJECT_NAME}_SRCS}
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME}
        PRIVATE
            $<$<AND:$<CONFIG:Debug>,$<BOOL:${AETHER_ENABLE_ASAN}>>:
                -fsanitize=address -fno-omit-frame-pointer -O1
            >
    )
    target_link_options(${PROJECT_NAME}
        PRIVATE
            $<$<CONFIG:Debug>:--coverage>
            $<$<AND:$<CONFIG:Debug>,$<BOOL:${AETHER_ENABLE_ASAN}>>:
                -fsanitize=address
            >
    )
# Add flags for other compilers here
endif()
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Aether
        GTest::gtest_main
        GTest::gmock
)
if(AETHER_ENABLE_ASAN)
    gtest_discover_tests(${PROJECT_NAME}
        DISCOVERY_MODE PRE_TEST
        PROPERTIES
            ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
    )
else()
    gtest_discover_tests(${PROJECT_NAME})
endif()
