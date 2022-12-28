cmake_minimum_required (VERSION 3.21)

set(THIRD_PARTY_LIBS Boost::boost 
                     Boost::filesystem
                     Boost::program_options
                     spdlog::spdlog
                     fmt::fmt)

set(TARGET_NAME assfonts)
						  
add_executable(${TARGET_NAME} assfonts.cc)

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS} libassfonts)

target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include
                                          PRIVATE ${THIRD_PARTY_INCLUDE_DIRS})

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

if(CMAKE_BUILD_TYPE MATCHES "Release")
    install(TARGETS ${TARGET_NAME})
endif()