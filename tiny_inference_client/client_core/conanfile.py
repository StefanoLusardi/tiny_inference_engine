from conans import ConanFile, CMake

class TinyInfereneceEngine_Client_Conan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"
    requires = "grpc/1.43.0"