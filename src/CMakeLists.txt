cmake_minimum_required (VERSION 3.20)

set(THIRD_PARTY_LIBS PRIVATE Boost::boost 
                             Boost::filesystem
                             spdlog::spdlog
                             CLI11::CLI11
                             fmt::fmt)
						  
add_executable(assfonts assfonts.cc)

target_link_libraries(assfonts PRIVATE -static libassfonts ${THIRD_PARTY_LIBS})

target_include_directories(assfonts PRIVATE ${PROJECT_SOURCE_DIR}/include
                                            ${VCPKG_INCLUDE_DIR})

if(MSVC)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(assfonts PRIVATE /W4 /WX /MDd /Od /D_WIN32_WINNT=0x0601)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        target_compile_options(assfonts PRIVATE /W4 /WX /MT /O2 /D_WIN32_WINNT=0x0601)
    endif()
endif()
