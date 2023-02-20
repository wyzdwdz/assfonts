from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, CMake, cmake_layout
from conan.tools.cmake.utils import is_multi_configuration
from conan.tools.system.package_manager import Apt, Yum, PacMan, Zypper
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

        if component is not None:
            arg_list.append("--component")
            arg_list.append(component)

        arg_list = " ".join(filter(None, arg_list))
        command = "%s %s" % (self._cmake_program, arg_list)
        self._conanfile.output.info("CMake command: %s" % command)
        self._conanfile.run(command)


class AssfontsConan(ConanFile):
    name = "assfonts"
    version = "0.3.2"

    license = "GNU General Public License v2.0"
    author = "wyzdwdz"
    url = "https://github.com/wyzdwdz/assfonts"
    description = "Subset fonts and embed them into an ASS subtitle"
    topics = ("SubStation Alpha", "Font subset")

    settings = ["os", "compiler", "build_type", "arch"]

    requires = ["harfbuzz/6.0.0",
                "freetype/2.12.1",
                "nlohmann_json/3.11.2",
                "libiconv/1.17",
                "spdlog/1.11.0",
                "fmt/9.1.0",
                "cli11/2.3.1",
                "pcre2/10.42"]

    default_options = {"harfbuzz:shared": False,
                       "harfbuzz:with_freetype": False,
                       "harfbuzz:with_icu": False,
                       "harfbuzz:with_glib": False,
                       "harfbuzz:with_subset": True,

                       "freetype:shared": False,
                       "freetype:with_png": False,
                       "freetype:with_zlib": False,
                       "freetype:with_bzip2": False,
                       "freetype:with_brotli": False,
                       "freetype:subpixel": False,

                       "libiconv:shared": False,

                       "spdlog:shared": False,
                       "spdlog:header_only": False,
                       "spdlog:no_exceptions": False,

                       "fmt:header_only": False,
                       "fmt:shared": False,

                       "pcre2:shared": False,
                       "pcre2:build_pcre2_8": True,
                       "pcre2:build_pcre2_16": False,
                       "pcre2:build_pcre2_32": False,
                       "pcre2:build_pcre2grep": False,
                       "pcre2:support_jit": True}

    def system_requirements(self):
        Apt(self).install_substitutes(["build-essential", "libgtk-3-dev"])
        Yum(self).install_substitutes(["gcc", "gcc-c++", "make", "gtk3-devel"])
        PacMan(self).install_substitutes(["base-devel", "gtk3"])
        Zypper(self).install_substitutes(["devel_basis", "gtk3-devel"])

    def config_options(self):
        if self.settings.os == 'Windows':
            self.options["harfbuzz"].with_gdi = False
            self.options["harfbuzz"].with_uniscribe = False
            self.options["harfbuzz"].with_directwrite = False

            self.options["spdlog"].wchar_support = True
            self.options["spdlog"].wchar_filenames = True

        elif self.settings.os == 'Linux':
            pass

        elif self.settings.os == 'Macos':
            pass

    def layout(self):
        cmake_layout(self)

    def generate(self):
        dp = CMakeDeps(self)
        dp.generate()
        tc = CMakeToolchain(self)
        tc.variables["VERSION_MAJOR"] = self.version.split('.')[0]
        tc.variables["VERSION_MINOR"] = self.version.split('.')[1]
        tc.variables["VERSION_PATCH"] = self.version.split('.')[2]
        tc.generate()

    def build(self):
        cmake = _CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()
