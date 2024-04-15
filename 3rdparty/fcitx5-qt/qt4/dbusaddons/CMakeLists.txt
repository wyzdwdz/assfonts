ecm_setup_version(PROJECT VARIABLE_PREFIX FCITX5QT4DBUSADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/fcitx5qt4dbusaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt4DBusAddonsConfigVersion.cmake"
                        SOVERSION 1)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/Fcitx5Qt4DBusAddons")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/Fcitx5Qt4DBusAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt4DBusAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

set(dbusaddons_SOURCES
    fcitxqtwatcher.cpp
    fcitxqtdbustypes.cpp
    fcitxqtinputcontextproxy.cpp
    fcitxqtinputcontextproxyimpl.cpp
    fcitxqtinputmethodproxy.cpp
    )

set(dbusaddons_HEADERS
    fcitxqtwatcher.h
    fcitxqtdbustypes.h
    fcitxqtinputcontextproxy.h
    fcitxqtinputmethodproxy.h
)

set(fcitxqtdbusaddons_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(Fcitx5Qt4DBusAddons ${LIBRARY_TYPE} ${dbusaddons_SOURCES})
generate_export_header(Fcitx5Qt4DBusAddons BASE_NAME Fcitx5Qt4DBusAddons)
add_library(Fcitx5Qt4::DBusAddons ALIAS Fcitx5Qt4DBusAddons)

target_include_directories(Fcitx5Qt4DBusAddons PUBLIC "$<BUILD_INTERFACE:${fcitxqtdbusaddons_INCLUDE_DIRS}>")
target_include_directories(Fcitx5Qt4DBusAddons INTERFACE "$<INSTALL_INTERFACE:${Fcitx5Qt4_INCLUDE_INSTALL_DIR}/Fcitx5Qt4DBusAddons>")

set_target_properties(Fcitx5Qt4DBusAddons
                      PROPERTIES VERSION ${FCITX5QT4DBUSADDONS_VERSION}
                      AUTOMOC TRUE
                      SOVERSION ${FCITX5QT4DBUSADDONS_SOVERSION}
                      POSITION_INDEPENDENT_CODE ON
                      EXPORT_NAME DBusAddons
)

target_link_libraries(
    Fcitx5Qt4DBusAddons
    PUBLIC
        Qt4::QtCore
        Qt4::QtDBus
)

if(BUILD_ONLY_PLUGIN)
  set_target_properties(Fcitx5Qt4DBusAddons PROPERTIES
                        COMPILE_DEFINITIONS "FCITX5QT4DBUSADDONS_STATIC_DEFINE")

else()
  install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt4DBusAddonsConfig.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt4DBusAddonsConfigVersion.cmake"
          DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
          COMPONENT Devel )

  install(EXPORT Fcitx5Qt4DBusAddonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE Fcitx5Qt4DBusAddonsTargets.cmake NAMESPACE Fcitx5Qt4:: )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx5qt4dbusaddons_version.h
          DESTINATION ${Fcitx5Qt4_INCLUDE_INSTALL_DIR} COMPONENT Devel )

  install(TARGETS Fcitx5Qt4DBusAddons EXPORT Fcitx5Qt4DBusAddonsTargets LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

  install(FILES ${dbusaddons_HEADERS}
                ${CMAKE_CURRENT_BINARY_DIR}/fcitx5qt4dbusaddons_export.h
                DESTINATION "${Fcitx5Qt4_INCLUDE_INSTALL_DIR}/Fcitx5Qt4DBusAddons")
endif()


