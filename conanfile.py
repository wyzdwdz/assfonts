from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout


class Assfonts(ConanFile):
    name = "assfonts"
    version = "0.5.0"

    license = "GNU General Public License v2.0"
    author = "wyzdwdz"
    url = "https://github.com/wyzdwdz/assfonts"
    description = "Subset fonts and embed them into an ASS subtitle"
    topics = ("SubStation Alpha", "Font subset")

    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("harfbuzz/7.3.0", force=True)
        self.requires("freetype/2.13.0")
        self.requires("nlohmann_json/3.11.2")
        self.requires("libiconv/1.17")
        self.requires("fmt/10.0.0")
        self.requires("cli11/2.3.2")
        self.requires("pcre2/10.42")
        self.requires("threadpool/20140926")
        self.requires("ghc-filesystem/1.5.14")
        self.requires("qt/5.15.9")

        self.requires("libpng/1.6.40", override=True)

    def build_requirements(self):
        self.tool_requires("cmake/3.26.3")

    def configure(self):
        self.options["*"].shared = False
        self.options["harfbuzz"].with_subset = True
        self.options["pcre2"].support_jit = True
        self.options["qt"].with_harfbuzz = True

        if self.settings.os == "Windows":
            self.options["libcurl"].with_ssl = "schannel"
        elif self.settings.os == "Macos":
            self.options["libcurl"].with_ssl = "darwinssl"

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