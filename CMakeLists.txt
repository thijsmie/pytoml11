cmake_minimum_required(VERSION 3.15...3.27)
set (CMAKE_CXX_STANDARD 20)
include(FetchContent)

# Setup Project
project(
  ${SKBUILD_PROJECT_NAME}
  VERSION ${SKBUILD_PROJECT_VERSION}
  LANGUAGES CXX
)

# Setup Python
find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

# Add the library
python_add_library(_value MODULE src/pytoml11.cpp WITH_SOABI)
target_link_libraries(_value PRIVATE pybind11::headers)
target_include_directories(_value PRIVATE src/)

# Output wheels
install(TARGETS _value DESTINATION pytoml11)
