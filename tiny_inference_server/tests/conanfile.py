from conans import ConanFile, CMake

class TinyInfereneceEngine_Server_Test_Conan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"
    requires = "gtest/1.10.0"