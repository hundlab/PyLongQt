====================
Top-level functions
====================

PyLongQt . **cellMap**: Dict[str, func]

A map from the name of each cell model to a constructor for that model. This is usually used with protocols to set the cell type or as a reference for the names of the different cells.

PyLongQt . **protoMap**: Dict[str, func]

A map from the name of each protocol to its constructor. Much like the cell map this adds an alternative way to create a Protocol without needing the Protocols module.

PyLongQt . **verson**: bool

Current version of PyLongQt (may be different from the current version of
LongQt.

PyLongQt . **max_print_lines**

Maxium length an object will display when printing to the prompt. This prevents excessively long objects from printing an overwhelming number of lines.

.. autofunction:: PyLongQt.verbose 

.. autoclass:: PyLongQt.Side
