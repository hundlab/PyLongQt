#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Mar 9 13:29:32 2020

@author: grat05
"""

import numpy as np

from ._PyLongQt import Structures

def simpleGrid(self):
    """
    Returns a simplified representation of the grid. Each node is replaced by a 
    number to make visualization easier. A lookup table between the cell's types
    and the number used in the simplified grid is also returned.
    """
    cell_types = {name: i 
        for i, name in enumerate(
        set(map(lambda x: x.cell.type, self))
        )}
    simple_grid = np.fromiter(
        map(lambda x: cell_types[x.cell.type], self), 
        dtype=int, 
        count=self.size)\
        .reshape(self.shape)
    return simple_grid, cell_types

Structures.Grid.simpleGrid = simpleGrid

