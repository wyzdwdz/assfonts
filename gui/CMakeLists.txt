cmake_minimum_required (VERSION 3.21)

set(THIRD_PARTY_LIBS PRIVATE wx::core
                             wx::base
                             Boost::boost
                             Boost::filesystem
                             Boost::thread
                             spdlog::spdlog)

set(TARGET_SOURCE PRIVATE gui_frame.cc
                          run.cc)

set(RESOURCE_DIR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resource)

set(INCLUDE_DIR PRIVATE ${PROJECT_SOURCE_DIR}/include
                        ${VCPKG_INCLUDE_DIR}
                        ${RESOURCE_DIR})

if(WIN32)
    LIST(APPEND TARGET_SOURCE ${RESOURCE_DIR}/assfonts_gui.rc
                              ${RESOURCE_DIR}/wx_dpi_aware.manifest)
endif()

set(TARGET_NAME assfonts_gui)

if(WIN32)						  
    add_executable(${TARGET_NAME} WIN32 assfonts_gui.cc)
else()
    add_executable(${TARGET_NAME} assfonts_gui.cc)
endif()

target_link_libraries(${TARGET_NAME} PRIVATE libassfonts ${THIRD_PARTY_LIBS})

target_include_directories(${TARGET_NAME} PRIVATE ${INCLUDE_DIR})

target_sources(${TARGET_NAME} PRIVATE ${TARGET_SOURCE})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(${TARGET_NAME} PRIVATE /Od /W4 /WX /MDd /D_WIN32_WINNT=0x0601)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        target_compile_options(${TARGET_NAME} PRIVATE /O2 /W4 /WX /MT /D_WIN32_WINNT=0x0601)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${TARGET_NAME} PRIVATE -no-pie)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(${TARGET_NAME} PRIVATE -O0 -Wall -Werror)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        target_compile_options(${TARGET_NAME} PRIVATE -O2 -Wall -Werror)
    endif()
endif()
