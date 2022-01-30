from conans import ConanFile, CMake

class TinyInfereneceEngine_Server_Conan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"
    requires = "grpc/1.43.0", "cli11/2.1.1", "spdlog/1.9.2"