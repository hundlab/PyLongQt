#-------------------------------------------------
#
# Project created by QtCreator 2015-10-19T09:57:24
#
#-------------------------------------------------

cmake_minimum_required(VERSION 3.0)

project(PyLongQt VERSION 0.2)

# Find includes in corresponding build directories
#Release Debug
set(CMAKE_BUILD_TYPE Release)

add_subdirectory(pybind11)
find_package(LongQt-model REQUIRED)
find_package(Qt5 COMPONENTS Core Concurrent REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PyLongQt_includes
    ${CMAKE_CURRENT_LIST_DIR}/src
)
include_directories(${PyLongQt_includes})
#get_target_property(qt_lib Qt5::Core INTERFACE_LINK_LIBRARIES)
#message("---")
#message("${qt_lib}")
#message("---")

set(PyLongQt_sources
        "src/pylongqt.cpp"
    "src/cells.cpp"
    "src/protocols.cpp"
    "src/structures.cpp"
    "src/misc.cpp"
)

#link_directories(${modellib_LIBRARY})
set(BUILD_QT_STATIC FALSE)

if(${BUILD_QT_STATIC})
#        link_directories("/usr/local/Qt-5.9.3/lib/")
	SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
	SET(BUILD_SHARED_LIBRARIES OFF)
#	SET(GCC_COVERAGE_LINK_FLAGS "-static")
endif()

add_library(PyLongQt MODULE ${PyLongQt_sources})
#if longqt-model is installed as a libaray
#find_library(lqmodel_LIBRARY NAMES LongQt-model HINTS ${modellib_LIBRARY})
#if(DEFINED lqmodel_LIBRARY)
#	message("-- LongQt-model found at " ${lqmodel_LIBRARY})
#endif()

if(${BUILD_QT_STATIC})
    #qt has a bug when its built statically where it doesn't link deps so
    if(WIN32 AND NOT CYGWIN)
        target_link_libraries(PyLongQt Qt5::Core Qt5::Concurrent libQt5EventDispatcherSupport.a libQt5FontDatabaseSupport.a libqtfreetype.a libQt5ThemeSupport.a libQt5AccessibilitySupport.a libQt5PrintSupport.a libqtlibpng.a libqtharfbuzz.a libQt5Concurrent.a libQt5Core.a libqtpcre2.a mingw32 shlwapi winspool uxtheme dwmapi comdlg32 oleaut32 imm32 glu32 opengl32 gdi32 mpr version ws2_32 kernel32 user32 shell32 uuid ole32 advapi32 winmm)
        target_compile_options(PyLongQt PRIVATE -static-libgcc -static-libstdc++)
    elseif(APPLE)
        set(QT_lib_dir ${Qt5_DIR}/../../)
        target_link_libraries(PyLongQt PRIVATE LongQt-model Qt5::Core Qt5::Concurrent pybind11::module ${QT_lib_dir}/libqtpcre2.a)# libQt5EventDispatcherSupport.a libQt5FontDatabaseSupport.a libqtfreetype.a libQt5ThemeSupport.a libQt5AccessibilitySupport.a libQt5PrintSupport.a libqtlibpng.a libqtharfbuzz.a libQt5Concurrent.a libQt5Core.a  mingw32 shlwapi winspool uxtheme dwmapi comdlg32 oleaut32 imm32 glu32 opengl32 gdi32 mpr version ws2_32 kernel32 user32 shell32 uuid ole32 advapi32 winmm)
        target_compile_options(PyLongQt PRIVATE -framework Cocoa)
    else()
        target_link_libraries(PyLongQt PUBLIC LongQt-model Qt5::Concurrent Qt5::Core pybind11::module)
    endif()
else()
     target_link_libraries(PyLongQt PUBLIC LongQt-model Qt5::Concurrent Qt5::Core pybind11::module)
endif()
set_target_properties(PyLongQt PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}"
                                         SUFFIX "${PYTHON_MODULE_EXTENSION}")

install(TARGETS PyLongQt DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
