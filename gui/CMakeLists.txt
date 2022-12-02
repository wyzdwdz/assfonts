cmake_minimum_required (VERSION 3.20)

set(THIRD_PARTY_LIBS PRIVATE wx::core
                             wx::base)

set(TARGET_SOURCE PRIVATE gui_frame.cc)

set(RESOURCE_DIR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resource)

set(INCLUDE_DIR PRIVATE ${PROJECT_SOURCE_DIR}/include
                        ${VCPKG_INCLUDE_DIR}
                        ${RESOURCE_DIR})

if(WIN32)
    LIST(APPEND TARGET_SOURCE ${RESOURCE_DIR}/assfonts_gui.rc
                              ${RESOURCE_DIR}/wx_dpi_aware.manifest)
endif()

if(WIN32)						  
    add_executable(assfonts_gui WIN32 assfonts_gui.cc)
else()
    add_executable(assfonts_gui assfonts_gui.cc)
endif()

target_link_libraries(assfonts_gui PRIVATE libassfonts ${THIRD_PARTY_LIBS})

target_include_directories(assfonts_gui PRIVATE ${INCLUDE_DIR})

target_sources(assfonts_gui PRIVATE ${TARGET_SOURCE})

if(MSVC)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(assfonts_gui PRIVATE /W4 /WX /MDd /Od /D_WIN32_WINNT=0x0601)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        target_compile_options(assfonts_gui PRIVATE /W4 /WX /MT /O2 /D_WIN32_WINNT=0x0601)
    endif()
endif()