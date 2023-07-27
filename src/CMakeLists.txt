cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS qt::qt)

set(TARGET_SOURCES main.cc
                   main_window.cc
                   task_runner.cc
                   log_highlighter.cc
                   check_window.cc
                   drop_lineedit.cc
                   checkable_button.cc)

if(WIN32)
    set(TARGET_RESOURCES resources/assfonts.rc
                         resources/assfonts.manifest)
elseif(APPLE)
    set(TARGET_RESOURCES resources/icon.icns)
    set_source_files_properties(${TARGET_RESOURCES} PROPERTIES
                                MACOSX_PACKAGE_LOCATION "Resources")
else()
endif()

qt5_add_resources(QT_RESOURCES resources/assfonts.qrc)

set(TARGET_NAME assfonts_gui)

set(BINARY_NAME assfonts)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
						  
add_executable(${TARGET_NAME} ${TARGET_SOURCES})

target_sources(${TARGET_NAME} PRIVATE ${QT_RESOURCES} ${TARGET_RESOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS} assfonts)

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})

set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${BINARY_NAME}
                                                WIN32_EXECUTABLE ON
                                                MACOSX_BUNDLE ON
                                                MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/resources/assfonts.plist)

if(APPLE)
    install(TARGETS ${TARGET_NAME}
            CONFIGURATIONS Release
            RUNTIME DESTINATION bin
            BUNDLE DESTINATION .)

    set(CPACK_PACKAGE_NAME ${BINARY_NAME})
    set(CPACK_PACKAGE_NAMECPACK_PACKAGE_VENDOR "wyzdwdz@gmail.com")
    set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
    set(CPACK_PACKAGE_DESCRIPTION "Subset fonts and embed them into an ASS subtitle")
    set(CPACK_PACKAGE_FILE_NAME ${BINARY_NAME})
    set(CPACK_STRIP_FILES TRUE)
    set(CPACK_PACKAGE_ICON ${PROJECT_SOURCE_DIR}/src/resources/icon.icns)
    set(CPACK_PACKAGE_DIRECTORY ${PROJECT_SOURCE_DIR})
    set(CPACK_GENERATOR "DragNDrop")
    include(CPack)
else()
    install(TARGETS ${TARGET_NAME} 
            CONFIGURATIONS Release 
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()