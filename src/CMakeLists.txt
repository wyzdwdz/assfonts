cmake_minimum_required (VERSION 3.20)

SET(third_party_libraries harfbuzz
						  harfbuzz-subset
						  freetype 
						  Boost::boost 
						  Boost::filesystem
						  Boost::locale
						  Boost::serialization
						  spdlog::spdlog
						  fmt::fmt
						  CLI11::CLI11)
						  
add_executable(assfonts "main.cc")

target_link_libraries(assfonts PRIVATE ${third_party_libraries})

target_include_directories(assfonts PRIVATE ${PROJECT_SOURCE_DIR}/include
											${VCPKG_INCLUDE_DIR})

target_sources(assfonts PRIVATE ass_parser.cc
								font_parser.cc
								font_subsetter.cc
								ass_font_embedder.cc)

if(MSVC)
	if(CMAKE_BUILD_TYPE MATCHES "Debug")
		target_compile_options(assfonts PRIVATE /MDd /Od)
	elseif(CMAKE_BUILD_TYPE MATCHES "Release")
		target_compile_options(assfonts PRIVATE /MT /O2)
	endif()
endif()
