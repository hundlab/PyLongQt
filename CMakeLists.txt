cmake_minimum_required(VERSION 3.16)

project(PyLongQt VERSION 0.2)

# Find includes in corresponding build directories
set(CMAKE_CXX_STANDARD 17)

# Try and find pybind11 and LongQt-model, if that fails, add them
# from subdirectory ./pybind11 and ../LongQt-model
find_package(pybind11)
if(NOT ${pybind11_FOUND})
  add_subdirectory(pybind11)
endif()

if(NOT DEFINED LongQt_subdir OR NOT ${LongQt_subdir})
  find_package(LongQt-model)
endif()

if((NOT DEFINED LongQt-model_FOUND) OR NOT ${LongQt-model_FOUND})
  if(EXISTS ../LongQt-model)
    add_subdirectory(../LongQt-model LongQt-model EXCLUDE_FROM_ALL)
  else()
    message(FATAL_ERROR "LongQt-model not found")
  endif() 
endif()


find_package(Qt5 COMPONENTS Core REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WIN32 AND TARGET Qt5::qmake AND NOT TARGET Qt5::windeployqt)
  get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(qt5_bin_location ${_qt5_qmake_location} DIRECTORY)
  if(EXISTS "${qt5_bin_location}/windeployqt.exe")
    add_executable(Qt5::windeployqt IMPORTED)
    set_target_properties(Qt5::windeployqt PROPERTIES
      IMPORTED_LOCATION "${qt5_bin_location}/windeployqt.exe")
  endif()
  message(STATUS "-- Qt5::windeployqt added")
elseif(APPLE AND TARGET Qt5::qmake AND NOT TARGET Qt5::macdeployqt)
  get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(qt5_bin_location ${_qt5_qmake_location} DIRECTORY)
  if(EXISTS "${qt5_bin_location}/macdeployqt")
    add_executable(Qt5::macdeployqt IMPORTED)
    set_target_properties(Qt5::macdeployqt PROPERTIES
      IMPORTED_LOCATION "${qt5_bin_location}/macdeployqt")
  endif()
  message(STATUS "-- Qt5::macdeployqt added")
endif()

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

## Use qt deploy tools to copy binaries
#		COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/libdeploy"
if(TARGET Qt5::windeployqt)
  add_custom_command(TARGET PyLongQt
    POST_BUILD
    COMMAND Qt5::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/libdeploy" "$<TARGET_FILE_DIR:PyLongQt>/$<TARGET_FILE_NAME:PyLongQt>"
  )
elseif(TARGET Qt5::macdeployqt)
#not working for mac
#  add_custom_command(TARGET PyLongQt
#    POST_BUILD
#    COMMAND Qt5::macdeployqt -executable="$<TARGET_FILE_DIR:PyLongQt>/$<TARGET_FILE_NAME:PyLongQt>"
#  )
endif()

install(TARGETS PyLongQt DESTINATION .)

### Install dependancies for windows and OSX
if(WIN32)
  install(
    DIRECTORY
    "${CMAKE_CURRENT_BINARY_DIR}/libdeploy/"
    DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
