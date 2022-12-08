cmake_minimum_required (VERSION 3.21)

set(THIRD_PARTY_LIBS Boost::boost 
                     Boost::filesystem
                     Boost::program_options
                     spdlog::spdlog
                     fmt::fmt)

set(TARGET_NAME assfonts)
						  
add_executable(${TARGET_NAME} assfonts.cc "fmt_sink.h")

target_link_libraries(${TARGET_NAME} PRIVATE libassfonts ${THIRD_PARTY_LIBS})

target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include
                                                  ${VCPKG_INCLUDE_DIR})

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})
