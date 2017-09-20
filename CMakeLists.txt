#-------------------------------------------------
#
# Project created by QtCreator 2015-10-19T09:57:24
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.8.11)

project(PyLongQt)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_TYPE Debug)
# Instruct CMake to run moc automatically when needed.
# Find the QtWidgets library
#link_directories("/Qt/Static/5.7.0/lib")
find_package(Qt5Core)
add_subdirectory(pybind11)
add_subdirectory(LongQt-model)

set(PyLongQt_includes
	${modellib_includes}
)
set(PyLongQt_headers
    "pylongqt.h"
	${modellib_headers}
)
set(PyLongQt_sources
	"pylongqt.cpp"
    "cells.cpp"
    "protocols.cpp"
    "structures.cpp"
    "misc.cpp"
	${modellib_sources}
)
include_directories(${PyLongQt_includes})
#link_directories(./build-modellib/release)
#message(${PyLongQt_sources})
#windows


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
set(CMAKE_AUTOMOC ON)
#add_custom_target(
#    modellib
#    COMMAND qmake ${CMAKE_CURRENT_SOURCE_DIR}/LongQt-model/modellib.pro
#    COMMAND make -f ${CMAKE_CURRENT_SOURCE_DIR}/LongQt-model/Makefile
#)

pybind11_add_module(PyLongQt ${PyLongQt_sources})
#add_dependencies(PyLongQt modellib)
#target_link_libraries(PyLongQt model)
target_link_libraries(PyLongQt Qt5::Core)
