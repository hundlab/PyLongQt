#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
*******************
1D Grid Simulation
*******************

This example illustrates how to setup and run a simple 1D grid simulation, 
also known as a fiber simulation with a row of cells. These simulations
show how the cell models interact rather than examining them in isolation. 2D
grids can be setup in the same fashion as 1D fibers, and there is also a more
advanced tutorial on seting up a 2D grid showing the possible customizations.
"""

#%%
# Setup & Run the Simulation
# ---------------------------
#
# Import PyLongQt and create a grid protocol, this protocol can function for
# 1 or 2D simulations.

import PyLongQt as pylqt


proto = pylqt.protoMap['Grid Protocol']()

#%%
# To configure the size of the Fiber we will add one row and the number of
# columns we would like to have for the simulation. Due to the detialed nature
# of many of the cell models, larger fibers/grids may be very slow and 
# computationally demanding.

proto.grid.addRow()
proto.grid.addColumns(10)

proto.grid

#%%
# When new cells are added to the grid, they are automatically filled with a 
# cell model called 'Inexcitable Cell', this cell acts like the edge of the
# grid. It is not excited by its neighbors and cannot pass any signal between
# them. To replace these cells with more interesting cells, we will create new
# cell objects and place them in the grid.

#%%
# To get the possible options for a node in a grid use

proto.grid[0,0].cellOptions()

#%%
# rather than

proto.cellOptions()

#%%
# as the cell cannot be set at the protocol level when there is a grid structure.
#
#    At this time all nodes will have the same options, it is not necessary to
#    check each node individually.

for col in range(proto.grid.columnCount()):
    node = proto.grid[0,col]
    node.setCellByName('Canine Ventricular (Hund-Rudy 2009)')

proto.grid


