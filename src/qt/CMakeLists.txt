cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS qt::qt
                     ghcFilesystem::ghc_filesystem
                     httplib::httplib)

set(TARGET_SOURCES main.cc
                   main_window.cc
                   task_runner.cc
                   log_highlighter.cc
                   check_window.cc
                   drop_lineedit.cc
                   checkable_button.cc)

if(WIN32)
    set(TARGET_RESOURCES resources/assfonts-gui.rc
                         resources/assfonts-gui.manifest)
elseif(APPLE)
    list(APPEND TARGET_SOURCES get_app_support_dir.mm)
    set(TARGET_RESOURCES resources/icon.icns)
    set_source_files_properties(${TARGET_RESOURCES} PROPERTIES
                                MACOSX_PACKAGE_LOCATION "Resources")
else()
endif()

qt5_add_resources(QT_RESOURCES resources/assfonts-gui.qrc)

set(TARGET_NAME assfonts-gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
						  
add_library(${TARGET_NAME} OBJECT ${TARGET_SOURCES})

target_sources(${TARGET_NAME} PRIVATE ${QT_RESOURCES} ${TARGET_RESOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS} libassfonts)

if(APPLE)
    target_link_options(${TARGET_NAME} PRIVATE -framework Foundation)
endif()

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTS})

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})

set_target_properties(${TARGET_NAME} PROPERTIES WIN32_EXECUTABLE ON
                                                MACOSX_BUNDLE ON
                                                MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/qt/resources/assfonts-gui.plist)

if(APPLE)
    install(TARGETS ${TARGET_NAME}
            CONFIGURATIONS Release
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            BUNDLE DESTINATION .)

    set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
    set(CPACK_PACKAGE_NAMECPACK_PACKAGE_VENDOR "wyzdwdz@gmail.com")
    set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
    set(CPACK_PACKAGE_DESCRIPTION "Subset fonts and embed them into an ASS subtitle")
    set(CPACK_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME})
    set(CPACK_STRIP_FILES TRUE)
    set(CPACK_PACKAGE_ICON ${PROJECT_SOURCE_DIR}/src/qt/resources/icon.icns)
    set(CPACK_PACKAGE_DIRECTORY ${PROJECT_SOURCE_DIR})
    set(CPACK_GENERATOR "DragNDrop")
    include(CPack)
else()
    install(TARGETS ${TARGET_NAME} 
            CONFIGURATIONS Release 
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
