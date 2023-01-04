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

        if component is not None:
            arg_list.append("--component")
            arg_list.append(component)

        arg_list = " ".join(filter(None, arg_list))
        command = "%s %s" % (self._cmake_program, arg_list)
        self._conanfile.output.info("CMake command: %s" % command)
        self._conanfile.run(command)


class AsshdrConan(ConanFile):
    name = "asshdr"
    version = "0.1.0"

    license = "GNU General Public License v2.0"
    author = "wyzdwdz"
    url = "https://github.com/wyzdwdz/asshdr"
    description = "Recolorize ASS subtitle for HDR contents"
    topics = ("SubStation Alpha", "HDR")

    settings = ["os", "compiler", "build_type", "arch"]

    requires = ["cli11/2.3.1",
                "pcre2/10.42"]

    default_options = {"fmt:shared": False,

                       "pcre2:build_pcre2_16": False,
                       "pcre2:build_pcre2_32": False,
                       "pcre2:build_pcre2_8": True,
                       "pcre2:build_pcre2grep": False,
                       "pcre2:grep_support_callout_fork": False,
                       "pcre2:shared": False,
                       "pcre2:support_jit": True,
                       "pcre2:with_bzip2": False,
                       "pcre2:with_zlib": False}

    def config_options(self):
        if self.settings.os == 'Windows':
            pass

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
        cmake.build(target="asshdr")
        cmake.install(component="asshdr")
