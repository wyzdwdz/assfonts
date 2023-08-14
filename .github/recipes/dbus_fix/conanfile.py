from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.gnu import PkgConfig
from conan.tools.system import package_manager

required_conan_version = ">=1.50.0"


class SysConfigEGLConan(ConanFile):
    name = "dbus"
    version = "system"
    description = "D-Bus is a simple system for interprocess communication and coordination."
    topics = "bus", "interprocess", "message"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://www.freedesktop.org/wiki/Software/dbus"
    license = ("AFL-2.1", "GPL-2.0-or-later")
    package_type = "shared-library"
    settings = "os", "arch", "compiler", "build_type"

    def layout(self):
        pass

    def package_id(self):
        self.info.clear()

    def validate(self):
        if self.settings.os not in ["Linux", "FreeBSD"]:
            raise ConanInvalidConfiguration("This recipes supports only Linux and FreeBSD")

    def system_requirements(self):
        dnf = package_manager.Dnf(self)
        dnf.install(["dbus-devel"], update=True, check=True)

        yum = package_manager.Yum(self)
        yum.install(["dbus-devel"], update=True, check=True)

        apt = package_manager.Apt(self)
        apt.install_substitutes(["libdbus-1-dev"], update=True, check=True)

        pacman = package_manager.PacMan(self)
        pacman.install(["dbus"], update=True, check=True)

        zypper = package_manager.Zypper(self)
        zypper.install(["dbus-1-devel"], update=True, check=True)

        pkg = package_manager.Pkg(self)
        pkg.install(["dbus"], update=True, check=True)

    def package_info(self):
        self.cpp_info.includedirs = []
        self.cpp_info.libdirs = []
        pkg_config = PkgConfig(self, "dbus-1")
        pkg_config.fill_cpp_info(self.cpp_info, is_system=True)