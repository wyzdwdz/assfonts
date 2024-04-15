cmake_minimum_required(VERSION 3.16)
project(fcitx5-qt VERSION 5.1.5)
set(FCITX5_QT_VERSION ${PROJECT_VERSION})

set(REQUIRED_QT4_VERSION 4.8.0)
set(REQUIRED_QT5_VERSION 5.5.0)
set(REQUIRED_QT6_VERSION 6.0.0)

find_package(ECM 1.4.0 REQUIRED)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(ENABLE_QT4 "Enable Qt 4" Off)
option(ENABLE_QT5 "Enable Qt 5" Off)
option(ENABLE_QT6 "Enable Qt 6" On)
option(ENABLE_X11 "Enable X11 support" On)
option(BUILD_ONLY_PLUGIN "Build only plugin" On)
option(BUILD_STATIC_PLUGIN "Build plugin as static" On)
option(WITH_FCITX_PLUGIN_NAME "Enable plugin name with fcitx" On)
option(ENABLE_QT6_WAYLAND_WORKAROUND "Enable Qt6 Wayland workaround" Off)

if (BUILD_ONLY_PLUGIN)
  set(LIBRARY_TYPE OBJECT)
else()
  set(LIBRARY_TYPE SHARED)
  add_definitions(-DFCITX_GETTEXT_DOMAIN=\"fcitx5-qt\")
  if (BUILD_STATIC_PLUGIN)
    message(FATAL_ERROR "BUILD_STATIC_PLUGIN requires BUILD_ONLY_PLUGIN")
  endif()
endif()

if (BUILD_ONLY_PLUGIN AND BUILD_STATIC_PLUGIN)
  set(PLUGIN_LIBRARY_TYPE STATIC)
else()
  set(PLUGIN_LIBRARY_TYPE MODULE)
endif()

add_definitions(-DFCITX5_QT_VERSION=\"${FCITX5_QT_VERSION}\")

include(GNUInstallDirs)
include(FeatureSummary)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMUninstallTarget)

if (ENABLE_X11)
    find_package(XCB REQUIRED COMPONENTS XCB)
endif()
find_package(XKBCommon 0.5.0 REQUIRED COMPONENTS XKBCommon)
if (NOT BUILD_ONLY_PLUGIN)
find_package(Fcitx5Utils 5.0.16 REQUIRED)
endif()

if (ENABLE_X11)
find_package(X11 REQUIRED)

add_library(X11Import UNKNOWN IMPORTED)
set_target_properties(X11Import PROPERTIES
    IMPORTED_LOCATION "${X11_X11_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${X11_X11_INCLUDE_PATH}")
endif()

if (BUILD_ONLY_PLUGIN)
# Dup a little bit option here.
include(CompilerSettings)
else()
include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")
find_package(Gettext REQUIRED)
endif()

set(Fcitx5Qt6_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/Fcitx5Qt6)
set(Fcitx5Qt5_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/Fcitx5Qt5)
set(Fcitx5Qt4_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/Fcitx5Qt4)

if(ENABLE_QT4)
  add_subdirectory(qt4)
endif()

if(ENABLE_QT5)
  add_subdirectory(qt5)
endif()

if(ENABLE_QT6)
  add_subdirectory(qt6)
endif()

if(NOT BUILD_ONLY_PLUGIN)
add_subdirectory(po)
endif()

enable_testing()
add_subdirectory(test)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
