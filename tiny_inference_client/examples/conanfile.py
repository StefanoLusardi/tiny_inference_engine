from conans import ConanFile, CMake

class TinyInfereneceEngine_ClientExamples_Conan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"
    requires = "opencv/4.5.5"