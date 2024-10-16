from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout, CMakeToolchain, CMakeDeps

class SimplicityRecipe(ConanFile):

    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("boost/1.86.0")
        self.requires("reflect-cpp/0.17.0")
        self.requires("libmysqlclient/8.1.0")
        self.requires("libpqxx/7.9.2")
        self.requires("cli11/2.4.2") # no need, remove later
        self.requires("nlohmann_json/3.11.3")
        self.requires("spdlog/1.15.1")
        self.requires("sol2/3.3.1")
        self.requires("catch2/3.7.0")
        
    def generate(self):
        tc = CMakeToolchain(self, generator='Ninja')
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build_requirements(self):
        self.tool_requires("cmake/3.31.3")

    def layout(self):
        cmake_layout(self, generator="Ninja")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()
