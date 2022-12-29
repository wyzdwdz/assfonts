from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, CMake, cmake_layout
from conan.tools.cmake.utils import is_multi_configuration
from conans.errors import ConanException


class _CMake(CMake):
    def install(self, build_type=None, component=None):
        bt = build_type or self._conanfile.settings.get_safe("build_type")
        if not bt:
            raise ConanException("build_type setting should be defined.")
        is_multi = is_multi_configuration(self._generator)
        build_config = "--config {}".format(bt) if bt and is_multi else ""

        build_folder = '"{}"'.format(self._conanfile.build_folder)
        arg_list = ["--install", build_folder, build_config]

        arg_list.append("--component")
        arg_list.append(component)

        arg_list = " ".join(filter(None, arg_list))
        command = "%s %s" % (self._cmake_program, arg_list)
        self._conanfile.output.info("CMake command: %s" % command)
        self._conanfile.run(command)


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
                       "harfbuzz:with_glib": False,

                       "freetype:shared": False,
                       "freetype:subpixel": False,
                       "freetype:with_brotli": False,
                       "freetype:with_bzip2": False,
                       "freetype:with_png": False,
                       "freetype:with_zlib": False,

                       "libiconv:shared": False,

                       "spdlog:shared": False,

                       "fmt:shared": False}

    def config_options(self):
        if self.settings.os == 'Windows':
            self.options["harfbuzz"].with_directwrite = False

            self.options["spdlog"].wchar_filenames = True
            self.options["spdlog"].wchar_support = True

        elif self.settings.os == 'Linux':
            pass

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
        cmake = _CMake(self)
        cmake.configure()
        cmake.build(target="assfonts")
        cmake.build(target="assfonts_gui")
        cmake.install(component="assfonts")
        cmake.install(component="assfonts_gui")
