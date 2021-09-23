#-------------------------------------------------
#
# Project created by QtCreator 2015-10-19T09:57:24
#
#-------------------------------------------------

cmake_minimum_required(VERSION 3.0)

project(PyLongQt VERSION 0.2)

# Find includes in corresponding build directories
set(CMAKE_CXX_STANDARD 17)


if(NOT DEFINED LongQt_subdir)
    set(LongQt_subdir FALSE)
endif()

add_subdirectory(pybind11)
if(NOT ${LongQt_subdir})
    find_package(LongQt-model REQUIRED)
else()
  add_subdirectory(../LongQt-model LongQt-model EXCLUDE_FROM_ALL)
endif()
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

set(PyLongQt_python
    "src/__init__.py"
    "src/_runsim.py"
    "src/_converters.py"
    "src/_grid.py"
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
install(FILES ${PyLongQt_python} DESTINATION ${CMAKE_INSTALL_PREFIX})
