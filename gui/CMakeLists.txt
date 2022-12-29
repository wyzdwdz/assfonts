cmake_minimum_required (VERSION 3.16)

set(TARGET_NAME assfonts_gui)

set(THIRD_PARTY_LIBS wx::core
                     wx::base
                     Boost::boost
                     Boost::filesystem
                     spdlog::spdlog)

set(TARGET_SOURCE gui_frame.cc
                  run.cc)

set(RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resource)

if(WIN32)
    LIST(APPEND TARGET_SOURCE ${RESOURCE_DIR}/assfonts_gui.rc
                              ${RESOURCE_DIR}/wx_dpi_aware.manifest)
endif()

set(INCLUDE_DIR ${RESOURCE_DIR})

if(WIN32)						  
    add_executable(${TARGET_NAME} WIN32 assfonts_gui.cc)
else()
    add_executable(${TARGET_NAME} assfonts_gui.cc)
endif()

target_sources(${TARGET_NAME} PRIVATE ${TARGET_SOURCE})

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS} libassfonts)

target_include_directories(${TARGET_NAME} PRIVATE ${INCLUDE_DIR}
                                                  ${PROJECT_SOURCE_DIR}/include
                                                  ${THIRD_PARTY_INCLUDE_DIRS})
                                                  

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

if(UNIX)
    target_link_options(${TARGET_NAME} PRIVATE -no-pie -pthread)
endif()

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${TARGET_NAME})