from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout


class Assfonts(ConanFile):
    name = "assfonts"
    version = "0.5.6"

    license = "GNU General Public License v3.0"
    author = "wyzdwdz"
    url = "https://github.com/wyzdwdz/assfonts"
    description = "Subset fonts and embed them into an ASS subtitle"
    topics = ("SubStation Alpha", "Font subset")

    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("harfbuzz/8.2.2", override=True)
        self.requires("freetype/2.13.0")
        self.requires("nlohmann_json/3.11.2")
        self.requires("libiconv/1.17")
        self.requires("fmt/10.0.0")
        self.requires("cli11/2.3.2")
        self.requires("pcre2/10.42")
        self.requires("threadpool/20140926")
        self.requires("ghc-filesystem/1.5.14")
        self.requires("string-view-lite/1.7.0")
        self.requires("qt/5.15.10")
        self.requires("cpp-httplib/0.13.3")

        self.requires("openssl/1.1.1u", override=True)
        self.requires("zlib/1.2.13", override=True)
        self.requires("libgettext/0.21", override=True)
        self.requires("libpng/1.6.40", override=True)

    def build_requirements(self):
        self.tool_requires("cmake/3.26.3")

    def configure(self):
        self.options["*"].shared = False
        self.options["harfbuzz"].with_subset = True
        self.options["pcre2"].support_jit = True

        self.options["qt"].with_harfbuzz = True
        self.options["qt"].with_pq = False
        self.options["qt"].with_mysql = False
        self.options["qt"].with_odbc = False
        self.options["qt"].with_sqlite3 = False
        if self.settings.os == "Linux":
            self.options["qt"].with_dbus = True
            self.requires("dbus/system", override=True)

        self.options["cpp-httplib"].with_openssl = True

    def layout(self):
        cmake_layout(self)

    def generate(self):
        dp = CMakeDeps(self)
        dp.generate()

        tc = CMakeToolchain(self)

        tc.variables["VERSION_MAJOR"] = self.version.split(".")[0]
        tc.variables["VERSION_MINOR"] = self.version.split(".")[1]
        tc.variables["VERSION_PATCH"] = self.version.split(".")[2]

        tc.generate()
