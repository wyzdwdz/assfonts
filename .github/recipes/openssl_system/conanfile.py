from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.gnu import PkgConfig
from conan.tools.system import package_manager

required_conan_version = ">=1.50.0"


class SysConfigOpenSSLConan(ConanFile):
    name = "openssl"
    version = "system"
    description = "A toolkit for the Transport Layer Security (TLS) and Secure Sockets Layer (SSL) protocols"
    topics = ("openssl", "ssl", "tls", "encryption", "security")
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://github.com/openssl/openssl"
    license = "OpenSSL"
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
        dnf.install(["openssl-devel"], update=True, check=True)

        yum = package_manager.Yum(self)
        yum.install(["openssl-devel"], update=True, check=True)

        apt = package_manager.Apt(self)
        apt.install_substitutes(["libssl-dev"], update=True, check=True)

        pacman = package_manager.PacMan(self)
        pacman.install(["openssl"], update=True, check=True)

        zypper = package_manager.Zypper(self)
        zypper.install(["libopenssl-devel"], update=True, check=True)

        pkg = package_manager.Pkg(self)
        pkg.install(["openssl"], update=True, check=True)

    def package_info(self):
        self.cpp_info.includedirs = []
        self.cpp_info.libdirs = []
        pkg_config = PkgConfig(self, "openssl")
        pkg_config.fill_cpp_info(self.cpp_info, is_system=True)