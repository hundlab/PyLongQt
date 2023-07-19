try:
  from PyQt5 import QtCore
except ModuleNotFoundError:
  pass

# import c++ PyLongQt functions
from ._PyLongQt import *
from . import _PyLongQt
__doc__ = _PyLongQt.__doc__

# We can extend c++ functionallity in python!
# Converters adds functions to read data directories
# and convert them
from . import _converters
from . import _runsim
from . import _grid
