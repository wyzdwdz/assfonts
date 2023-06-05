from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain


class Assfonts(ConanFile):
    name = "assfonts"
    version = "0.4.1"

    license = "GNU General Public License v2.0"
    author = "wyzdwdz"
    url = "https://github.com/wyzdwdz/assfonts"
    description = "Subset fonts and embed them into an ASS subtitle"
    topics = ("SubStation Alpha", "Font subset")

    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("harfbuzz_expt/7.3.0")
        self.requires("freetype/2.13.0")
        self.requires("nlohmann_json/3.11.2")
        self.requires("libiconv/1.17")
        self.requires("fmt/10.0.0")
        self.requires("cli11/2.3.2")
        self.requires("pcre2/10.42")
        self.requires("threadpool/20140926")
        self.requires("glfw/3.3.8")
        self.requires("ghc-filesystem/1.5.14")
        self.requires("libcurl/8.1.1")

    def build_requirements(self):
        self.tool_requires("cmake/3.26.3")

    def configure(self):
        self.options["*"].shared = False
        self.options["harfbuzz_expt"].with_subset = True
        self.options["pcre2"].support_jit=True

        if self.settings.os == "Windows":
            self.options["libcurl"].with_ssl = "schannel"
        elif self.settings.os == "Macos":
            self.options["libcurl"].with_ssl = "darwinssl"

    def layout(self):
        self.folders.build = f"build/{str(self.settings.build_type)}"
        self.folders.generators = "build"

    def generate(self):
        dp = CMakeDeps(self)
        dp.generate()

        tc = CMakeToolchain(self)

        tc.variables["VERSION_MAJOR"] = self.version.split('.')[0]
        tc.variables["VERSION_MINOR"] = self.version.split('.')[1]
        tc.variables["VERSION_PATCH"] = self.version.split('.')[2]
        
        tc.generate()