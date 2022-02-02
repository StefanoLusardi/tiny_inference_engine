from conans import ConanFile, CMake

class TinyInfereneceEngine_ServerExamples_Conan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"
    requires = "opencv/4.5.3"