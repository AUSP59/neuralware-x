# SPDX-License-Identifier: Apache-2.0
from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

class NWXConan(ConanFile):
    name = "neuralwarex"
    version = "0.2.0"
    license = "Apache-2.0"
    description = "Verifiable neural networks in C++"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "CMakeLists.txt", "src/*", "include/*", "cmake/*"
    def layout(self): cmake_layout(self)
    def build(self):
        cm = CMake(self); cm.configure(); cm.build()
    def package(self):
        cm = CMake(self); cm.install()
    def package_info(self):
        self.cpp_info.libs = ["nwx"]
