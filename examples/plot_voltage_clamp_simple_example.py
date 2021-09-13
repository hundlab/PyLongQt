#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
*********************
Simple Voltage Clamp
*********************

This example illustrates how to setup and run a voltage clamp simulation.
These simulations hold the cell's transmembrane voltage `vOld` to a fixed value
for specified lengths of time. They are most useful for measuring the cell's 
response to specific voltages. This example assumes that you have gone through
the simple current clamp example which explains the general LongQt simulation
setup.
"""

import PyLongQt as pylqt

proto = pylqt.protoMap['Voltage Clamp Protocol']()
proto.setCellByName('Human Ventricular (Ten Tusscher 2004)')

proto.tMax = 150
proto.meastime = 0
proto.writetime = 0

varSel = proto.cell.variableSelection
varSel.add('iNa')
proto.cell.variableSelection = varSel

proto.measureMgr.addMeasure('iNa', {'min', 'avg'})

proto.clamps.insert(0, -80)
proto.clamps.insert(100, 10)
proto.clamps.insert(120, -40)

sim_runner = pylqt.RunSim()
sim_runner.setSims(proto)
sim_runner.run()
sim_runner.wait()

