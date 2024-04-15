ecm_setup_version(PROJECT VARIABLE_PREFIX FCITX5QT5WIDGETSADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/fcitx5qt5widgetsaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt5WidgetsAddonsConfigVersion.cmake"
                        SOVERSION 2)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/Fcitx5Qt5WidgetsAddons")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/Fcitx5Qt5WidgetsAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt5WidgetsAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt5WidgetsAddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt5WidgetsAddonsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT Fcitx5Qt5WidgetsAddonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE Fcitx5Qt5WidgetsAddonsTargets.cmake NAMESPACE Fcitx5Qt5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx5qt5widgetsaddons_version.h
        DESTINATION ${Fcitx5Qt5_INCLUDE_INSTALL_DIR} COMPONENT Devel )

set(widgetsaddons_SOURCES
    fcitxqtconfiguifactory.cpp
    fcitxqtconfiguiplugin.cpp
    fcitxqtconfiguiwidget.cpp
    fcitxqtkeysequencewidget.cpp
    qtkeytrans.cpp
    )

set(widgetsaddons_HEADERS
    fcitxqtconfiguifactory.h
    fcitxqtconfiguiplugin.h
    fcitxqtconfiguiwidget.h
    fcitxqtkeysequencewidget.h
    fcitxqti18nhelper.h
)

set(fcitxqtwidgetsaddons_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${FCITX54_FCITX5_UTILS_INCLUDE_DIRS}
)

add_library(Fcitx5Qt5WidgetsAddons SHARED ${widgetsaddons_SOURCES})
generate_export_header(Fcitx5Qt5WidgetsAddons BASE_NAME Fcitx5Qt5WidgetsAddons)
add_library(Fcitx5Qt5::WidgetsAddons ALIAS Fcitx5Qt5WidgetsAddons)

target_include_directories(Fcitx5Qt5WidgetsAddons PUBLIC "$<BUILD_INTERFACE:${fcitxqtwidgetsaddons_INCLUDE_DIRS}>")
target_include_directories(Fcitx5Qt5WidgetsAddons INTERFACE "$<INSTALL_INTERFACE:${Fcitx5Qt5_INCLUDE_INSTALL_DIR}/Fcitx5QtWidgetsAddons>")

set_target_properties(Fcitx5Qt5WidgetsAddons
                      PROPERTIES VERSION ${FCITX5QT5WIDGETSADDONS_VERSION}
                      AUTOMOC TRUE
                      SOVERSION ${FCITX5QT5WIDGETSADDONS_SOVERSION}
                      EXPORT_NAME WidgetsAddons
)

target_link_libraries(Fcitx5Qt5WidgetsAddons
    PUBLIC
        Fcitx5::Utils
        Qt5::Core
        Qt5::Widgets
)

install(TARGETS Fcitx5Qt5WidgetsAddons EXPORT Fcitx5Qt5WidgetsAddonsTargets LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(FILES ${widgetsaddons_HEADERS}
              ${CMAKE_CURRENT_BINARY_DIR}/fcitx5qt5widgetsaddons_export.h
              DESTINATION "${Fcitx5Qt5_INCLUDE_INSTALL_DIR}/Fcitx5QtWidgetsAddons")
