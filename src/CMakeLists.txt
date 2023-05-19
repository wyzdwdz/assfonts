cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS ImGuiFileDialog
                     imgui
                     glfw)

set(TARGET_SOURCES main.cc
                   backends/imgui_impl_glfw.cpp
                   backends/imgui_impl_opengl3.cpp)

set(TARGET_RESOURCES )

set(TARGET_NAME assfonts_gui)

set(BINARY_NAME assfonts)
						  
add_executable(${TARGET_NAME} ${TARGET_SOURCES} ${TARGET_RESOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS} assfonts)

target_include_directories(${TARGET_NAME} PRIVATE backends
                                                  resources)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_BUILD_TYPE MATCHES "Release")
        target_link_options(${TARGET_NAME} PRIVATE -s)
    endif()
endif()

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})

set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${BINARY_NAME})

install(TARGETS ${TARGET_NAME} 
        CONFIGURATIONS Release 
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
