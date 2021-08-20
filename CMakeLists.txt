#-------------------------------------------------
#
# Project created by QtCreator 2015-10-19T09:57:24
#
#-------------------------------------------------

cmake_minimum_required(VERSION 3.0)

project(PyLongQt VERSION 0.2)

# Find includes in corresponding build directories
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(pybind11)
find_package(LongQt-model REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PyLongQt_includes
    ${CMAKE_CURRENT_LIST_DIR}/src
)
include_directories(${PyLongQt_includes})

set(PyLongQt_sources
        "src/pylongqt.cpp"
    "src/cells.cpp"
    "src/protocols.cpp"
    "src/structures.cpp"
    "src/measures.cpp"
    "src/pvars.cpp"
)

#link_directories(${modellib_LIBRARY})

add_library(PyLongQt MODULE ${PyLongQt_sources})

target_link_libraries(PyLongQt PUBLIC LongQt-model Qt5::Core pybind11::module)

set_target_properties(PyLongQt PROPERTIES 
	PREFIX "${PYTHON_MODULE_PREFIX}"
        SUFFIX "${PYTHON_MODULE_EXTENSION}"
	OUTPUT_NAME "_PyLongQt"
)

install(TARGETS PyLongQt DESTINATION ${CMAKE_INSTALL_PREFIX})
