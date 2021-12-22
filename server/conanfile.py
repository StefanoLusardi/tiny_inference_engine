from conans import ConanFile, CMake

class ServerEngineConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"
    requires = "grpc/1.39.1", "cli11/2.1.1", "spdlog/1.9.2" # comma-separated list of requirements

    def imports(self):
        self.copy("*.so*", dst="../lib", src="lib")
        self.copy("*.dll", dst="../bin", src="bin") 
        self.copy("*.dylib*", dst="../bin", src="lib")