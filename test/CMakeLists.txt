cmake_minimum_required (VERSION 3.21)

set(THIRD_PARTY_LIBS Boost::boost 
                     Boost::filesystem
                     Boost::test
                     spdlog::spdlog
                     fmt::fmt)

set(TARGET_NAME ass_test)

add_executable(${TARGET_NAME} test.cc)

target_include_directories(${TARGET_NAME} PRIVATE ${VCPKG_INCLUDE_DIR})