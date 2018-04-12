#-------------------------------------------------
#
# Project created by QtCreator 2015-10-19T09:57:24
#
#-------------------------------------------------

cmake_minimum_required(VERSION 2.8.11)

project(PyLongQt)

# Find includes in corresponding build directories
set(CMAKE_BUILD_TYPE Release)

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a")
add_subdirectory(pybind11)
add_subdirectory(LongQt-model)
find_package(Qt5Core)
set(CMAKE_AUTOMOC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PyLongQt_includes
	${modellib_includes}
)
include_directories(${PyLongQt_includes})

set(PyLongQt_sources
	"pylongqt.cpp"
    "cells.cpp"
    "protocols.cpp"
    "structures.cpp"
    "misc.cpp"
	${modellib_sources}
)

if(${BUILD_QT_STATIC})
	link_directories(${QT_LIB_DIR})
	SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
	SET(BUILD_SHARED_LIBRARIES OFF)
	SET(GCC_COVERAGE_LINK_FLAGS "-static")
endif()

pybind11_add_module(PyLongQt ${PyLongQt_sources})
#if longqt-model is installed as a libaray
#find_library(lqmodel_LIBRARY NAMES LongQt-model HINTS ${modellib_LIBRARY})
#if(DEFINED lqmodel_LIBRARY)
#	message("-- LongQt-model found at " ${lqmodel_LIBRARY})
#endif()
#target_link_libraries(PyLongQt ${lqmodel_LIBRARY})

if(${BUILD_QT_STATIC})
	target_link_libraries(PyLongQt Qt5::Core libQt5EventDispatcherSupport.a libQt5FontDatabaseSupport.a libqtfreetype.a libQt5ThemeSupport.a libQt5AccessibilitySupport.a libQt5PrintSupport.a libqtlibpng.a libqtharfbuzz.a libQt5Concurrent.a libQt5Core.a libqtpcre2.a mingw32 shlwapi winspool uxtheme dwmapi comdlg32 oleaut32 imm32 glu32 opengl32 gdi32 mpr version ws2_32 kernel32 user32 shell32 uuid ole32 advapi32 winmm) 
	#target_link_libraries(PyLongQt Qt5::Core C:/Qt/Static/5.9.3/plugins/printsupport/libwindowsprintersupport.a C:/Qt/Static/5.9.3/plugins/platforms/libqwindows.a libQt5EventDispatcherSupport.a libQt5FontDatabaseSupport.a libqtfreetype.a libQt5ThemeSupport.a libQt5AccessibilitySupport.a C:/Qt/Static/5.9.3/plugins/imageformats/libqgif.a C:/Qt/Static/5.9.3/plugins/imageformats/libqicns.a C:/Qt/Static/5.9.3/plugins/imageformats/libqico.a C:/Qt/Static/5.9.3/plugins/imageformats/libqjpeg.a C:/Qt/Static/5.9.3/plugins/imageformats/libqtga.a C:/Qt/Static/5.9.3/plugins/imageformats/libqtiff.a C:/Qt/Static/5.9.3/plugins/imageformats/libqwbmp.a C:/Qt/Static/5.9.3/plugins/imageformats/libqwebp.a libQt5PrintSupport.a  libQt5Widgets.a libQt5Gui.a libqtlibpng.a libqtharfbuzz.a libQt5Concurrent.a libQt5Core.a libqtpcre2.a mingw32 shlwapi winspool uxtheme dwmapi comdlg32 oleaut32 imm32 glu32 opengl32 gdi32 mpr version ws2_32 kernel32 user32 shell32 uuid ole32 advapi32 winmm) 
	target_compile_options(PyLongQt PRIVATE -static-libgcc -static-libstdc++)
else()
	target_link_libraries(PyLongQt Qt5::Core)
endif()

install(TARGETS PyLongQt DESTINATION ${INSTALL_LOC})
