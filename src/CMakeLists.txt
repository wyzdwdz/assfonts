cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS ImGuiFileDialog
                     imgui
                     glfw
                     ghcFilesystem::ghc_filesystem
                     CURL::libcurl)

set(TARGET_SOURCES main.cc
                   backends/imgui_impl_glfw.cpp
                   backends/imgui_impl_opengl3.cpp)

if(WIN32)
    set(TARGET_RESOURCES resources/assfonts.rc
                         resources/assfonts.manifest)
endif()

set(TARGET_NAME assfonts_gui)

set(BINARY_NAME assfonts)
						  
add_executable(${TARGET_NAME} ${TARGET_SOURCES} ${TARGET_RESOURCES})

if(APPLE)
    set_target_properties(${TARGET_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/resources/assfonts.plist)
endif()

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS} assfonts)

target_include_directories(${TARGET_NAME} PRIVATE backends
                                                  resources)

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})

set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${BINARY_NAME})

if(APPLE)
    install(TARGETS ${TARGET_NAME}
            CONFIGURATIONS Release
            RUNTIME DESTINATION bin
            BUNDLE DESTINATION .)

    set(APPS "\${CMAKE_INSTALL_PREFIX}/${BINARY_NAME}.app")
    set(DIRS ${CMAKE_BINARY_DIR})
    set(CPACK_GENERATOR "DragNDrop")
    include(CPack)
else()
    install(TARGETS ${TARGET_NAME} 
            CONFIGURATIONS Release 
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
