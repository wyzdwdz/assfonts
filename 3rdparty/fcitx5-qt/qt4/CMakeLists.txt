find_package(Qt4 ${REQUIRED_QT4_VERSION} REQUIRED)

add_subdirectory(dbusaddons)
add_subdirectory(inputcontext)

if(NOT BUILD_ONLY_PLUGIN)
  add_subdirectory(immodule-probing)
endif()
