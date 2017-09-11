#-------------------------------------------------
#
# Project created by QtCreator 2015-10-19T09:57:24
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.8.11)

project(modellib)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

include_directories(modellib modellib/cell modellib/protocol modellib/structure)
link_directories(~/src/LongQt/build)
# Find the QtWidgets library
find_package(Qt5Core)
add_subdirectory(pybind11)

# Tell CMake to create the helloworld executable
#add_executable(helloworld WIN32 main.cpp)

# Use the Widgets module from Qt 5.
#target_link_libraries(helloworld Qt5::Core)

#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++0x")
#set(SOURCES
#    main.cpp
#)
#SET(GCC_COVERAGE_COMPILE_FLAGS "-DQT_NO_KEYWORDS")
#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
#SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
pybind11_add_module(PyLongQt modellib.cpp)
target_link_libraries(PyLongQt Qt5::Core)
target_link_libraries(PyLongQt model-debug)
