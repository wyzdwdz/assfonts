cmake_minimum_required (VERSION 3.16)

set(wxBUILD_SHARED OFF)
set(wxBUILD_TESTS OFF)
set(wxBUILD_SAMPLES OFF)
set(wxBUILD_DEMOS OFF)
set(wxUSE_GUI ON)
set(wxBUILD_COMPATIBILITY 3.0)
set(wxBUILD_PRECOMP ON)
set(wxBUILD_MONOLITHIC OFF)

add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/compact_enc_det)
add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/wxWidgets)