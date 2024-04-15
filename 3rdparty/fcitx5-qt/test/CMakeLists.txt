if (TARGET Fcitx5Qt5::WidgetsAddons)

add_executable(testkeytrans testkeytrans.cpp)
target_link_libraries(testkeytrans Fcitx5Qt5::WidgetsAddons)
add_test(testkeytrans testkeytrans)

endif()
