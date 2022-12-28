from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, CMake, cmake_layout


class AssfontsConan(ConanFile):
    name = "assfonts"
    version = "0.2.6"

    license = "GNU General Public License v2.0"
    author = "wyzdwdz"
    url = "https://github.com/wyzdwdz/assfonts"
    description = "Subset fonts and embed them into an ASS subtitle"
    topics = ("SubStation Alpha", "Font subset")

    settings = ["os", "compiler", "build_type", "arch"]

    requires = ["harfbuzz/6.0.0",
                "freetype/2.12.1",
                "boost/1.80.0",
                "libiconv/1.17",
                "spdlog/1.11.0",
                "fmt/9.1.0"]

    default_options = {"harfbuzz:shared": False,
                       "harfbuzz:with_subset": True,

                       "freetype:shared": False,
                       "freetype:subpixel": False,
                       "freetype:with_brotli": False,
                       "freetype:with_bzip2": False,
                       "freetype:with_png": False,
                       "freetype:with_zlib": False,

                       "libiconv:shared": False,

                       "spdlog:shared": False,

                       "fmt:shared": False}

    def configure(self):
        if self.settings.os == 'Windows':
            self.options["harfbuzz"].with_directwrite = False

            self.options["spdlog"].wchar_filenames = True
            self.options["spdlog"].wchar_support = True

        elif self.settings.os == 'Linux':
            self.options["harfbuzz"].with_glib = False

        elif self.settings.os == 'Macos':
            self.options["harfbuzz"].with_uniscribe = False

    def layout(self):
        cmake_layout(self)

    def generate(self):
        dp = CMakeDeps(self)
        dp.generate()
        tc = CMakeToolchain(self)
        tc.generate()
        
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build(target="assfonts")
        cmake.build(target="assfonts_gui")

    def package(self):
        cmake = CMake(self)
        cmake.install()
