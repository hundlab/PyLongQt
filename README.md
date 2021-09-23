# PyLongQt

Python bindings for LongQt, a cardiac cell electrophysiology simulation platform.
This library allows for cell models to be created and configured to run both simple
and complex simulations.

*Hundlab Website: [hundlab.org](http://hundlab.org/)*

*PyPI: [pypi.org/project/hundlab-PyLongQt](https://pypi.org/project/hundlab-PyLongQt/)*

*Github: [github.com/hundlab/PyLongQt](https://github.com/hundlab/PyLongQt)*

*Documentation:*

 - *The PyLongQt API can be found in the [PyLongQt manual](https://hundlab.github.io/PyLongQt)
   along with some examples*
 - *[LongQt manual](http://longqt.readthedocs.io) demonstrates how the LongQt gui while also
   providing further details.*

## Installation

PyLongQt is a package on PyPI named hundlab_pylongqt and can be installed using
`python -m pip install hundlab_pylongqt`.

## Developer Instructions

There are two ways to build PyLongQt, one produces a package that can be installed using pip
the other uses cmake to install the library. The pip package install is best for active
development as it allows for a simpler build and install setup with QtCreator or another IDE.
The pip build is for producing a package that can be installed in python, this is used by
the github workflow system to build the pacakges distributed on PyPI.

### Development Build

To start both the PyLongQt project and LongQt-model project need to be cloned from github into
the following directory structure

```
<parent_dir>/LongQt-model
<parent_dir>/PyLongQt
```

Be sure to recursively clone PyLongQt so that pybind11 is cloned as a subdirectory. PyLongQt needs
to be built with different build systems depending on the platform, for windows use MSVC 2019,
for linux use gcc (or clang), and for mac use clang. On windows MSCV 2019 can be installed as
a standalone package or as a part of the visual studio IDE. Qt5 Core needs to be installed, and
optionally QtCreator.

To build PyLongQt, cmake needs to be told to look for LongQt-model as a subdirectory, this is why
the directory structure needs to follow the description above. When running cmake, specify
`LongQt_subdir:BOOL=TRUE`. If using QtCreator this can be done through the projects tab, if using
the command line, it can be specified as an argument to cmake `-DLongQt_subdir:BOOL=TRUE`.

To install PyLongQt add the install target to the build process (in QtCreator) or run cmake
with the install target (for command line). It is often best to set the install directory
manually, I install into a install subdirectory in my home folder. Set the CMAKE_INSTALL_PREFIX,
and ensure that it ends in PyLongQt. Once installed, use pip to manually install the dependancies
found in the `requirements.txt` file (`python -m pip install -r requirements.txt`).
Finally, your installation directory needs to be added
to the PYTHONPATH. For mac and linux this can be done by setting it in your .bash_profile (or corresponding
shell profile file). On windows the setting can be found in `System Properties -> Advanced -> Environment Variables`
and add `PYTHONPATH` with the location of your install directory (the parent of the PyLongQT install
location). Now PyLongQt should be ready to use. After making changes to the code, the project
just needs rebuild and reinstalled. Windows Note: When rebuilding PyLongQt it cannont be currently imported
anywhere, so python interpreters may need to be restarted if PyLongQt has been imported (only an issue
on windows, though on other platforms the python interpreters may still need to be restarted to allow
for PyLongQt to be reimported).

### Package Build

This build requires scikit-build to be installed using pip. For this build I do not use QtCreator,
although it may be possible to configure it to work. To build open a terminal (on windows use
`Developer Command Prompt for VS 2019`) and navagate to the PyLongQt github directory, then
run `python setup.py build bdist_wheel -- -DLongQt_subdir:BOOL=TRUE` on windows you may need to
specify the generator as well by adding `-G "Visual Studio 16 2019"`.

Once built, a wheel file (.whl) will be created in PyLongQt/dist. This file can be installed by
running using pip by passing in the full file name (ending in .whl) to pip install. If rebuilding
PyLongQt it will be necessary to uninstall the pacakge using `pip uninstall hundlab_pylongqt` before
attempting to reinstall.
