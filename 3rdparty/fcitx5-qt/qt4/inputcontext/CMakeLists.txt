
set(plugin_SRCS
    qfcitxinputcontext.cpp
    qtkey.cpp
    main.cpp
)

if (BUILD_ONLY_PLUGIN AND BUILD_STATIC_PLUGIN)
set(plugin_SRCS ${plugin_SRCS} $<TARGET_OBJECTS:Fcitx5Qt4DBusAddons>)
endif()


add_library(qtim-fcitx5 ${PLUGIN_LIBRARY_TYPE} ${plugin_SRCS})
set_target_properties(qtim-fcitx5 PROPERTIES
                         AUTOMOC TRUE
                        )
target_include_directories(qtim-fcitx5 PRIVATE "${PROJECT_SOURCE_DIR}/common")
if (BUILD_ONLY_PLUGIN AND BUILD_STATIC_PLUGIN)
    target_compile_definitions(qtim-fcitx5 PRIVATE "-DQT_STATICPLUGIN")
endif()
if (WITH_FCITX_PLUGIN_NAME)
    target_compile_definitions(qtim-fcitx5 PRIVATE "-DFCITX5_QT_WITH_FCITX_NAME")
endif()

target_link_libraries(qtim-fcitx5
                          Qt4::QtCore
                          Qt4::QtGui
                          Qt4::QtDBus
                          Fcitx5Qt4::DBusAddons
                          XKBCommon::XKBCommon
                         )

if (ENABLE_X11)
    target_link_libraries(qtim-fcitx5 X11Import)
    target_compile_definitions(qtim-fcitx5 PRIVATE "-DENABLE_X11")
endif()

set(CMAKE_INSTALL_QT4PLUGINDIR ${QT_PLUGINS_DIR} CACHE PATH "Qt4 plugin dir")
install(TARGETS qtim-fcitx5 DESTINATION ${CMAKE_INSTALL_QT4PLUGINDIR}/inputmethods)
