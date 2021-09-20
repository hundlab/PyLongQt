#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
************************
2. Simple Voltage Clamp
************************

This example illustrates how to setup and run a voltage clamp simulation.
These simulations hold the cell's transmembrane voltage `vOld` to a fixed value
for specified lengths of time. They are most useful for measuring the cell's 
response to specific voltages. This example assumes that you have gone through
the simple current clamp example which explains the general LongQt simulation
setup.
"""

#%%
# Setup & Run the Simulation
# ---------------------------

import PyLongQt as pylqt

proto = pylqt.protoMap['Voltage Clamp Protocol']()
proto.setCellByName('Human Ventricular (Ten Tusscher 2004)')

#%%
# The initial setup is similar to the current clamp setup with the exception
# that we create a voltage clamp protocol instead. Optionally we could set 
# cell options if that is desired. The voltage clamp protocol can be configured
# using the protocol's settings, the trace's variable selection, and the
# measure's selection.

proto.tMax = 150
proto.meastime = 0
proto.writetime = 0


varSel = proto.cell.variableSelection
varSel.add('iNa')
proto.cell.variableSelection = varSel

proto.measureMgr.addMeasure('iNa', {'min', 'avg'})

#%%
# One difference from the current clamp simulation is that the measure will
# take measurements for every voltage clamp period, rather than for each
# beat. To configure which voltage clamps will be applied we use the 
# :py:data:`Protocol.clamps`. The `clamps` object is a list of start times
# to apply a voltage, and the voltage to apply at that time until the next time.

proto.clamps.insert(0, -80)

#%%
# First we insert a clamp starting at time 0 (ms) which will clamp the cell
# membrane voltage :py:data:`Cell.vOld` to a voltage of -80 (mV). This clamp
# will be applied until the another clamp takes effect.

proto.clamps.insert(100, 10)
proto.clamps.insert(120, -40)
list(proto.clamps)

#%%
# These are the clamps we have added, they will always be displayed in order
# by their start times. Now the simulation is all setup and can be run.

sim_runner = pylqt.RunSim()
sim_runner.setSims(proto)
sim_runner.run()
sim_runner.wait()


#%%
#
# Plot the Results
# ----------------
#
# Reading and plotting the saved data can be preformed in the same manor
# as for the current clamp simulation.


import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

[trace], [meas] = pylqt.DataReader.readAsDataFrame(proto.datadir)

#%%
# The voltage plot shows is that voltage is being clamped to the desired
# values.

plt.figure()
plt.plot('t', 'vOld', data=trace)
plt.xlabel('Time (ms)')
plt.ylabel('Voltage (mV)')

#%%
# The measurements are recorded once per clamp, regardless of whether the
# cell had a response. Looking at the measurements for the sodium current, 
# we can see that only the 10 mV clamp at 100 ms had a response

clamp_n = (np.arange(meas.shape[0])+1).astype('str')

plt.figure('Peak Sodium Current vs Beat')
plt.bar(clamp_n, meas[('iNa', 'min')])
plt.ylabel('Peak Sodium Current (pA/pF)')
plt.xlabel('Clamp')

#%%
# The voltage of +10 mV elicited a current response from the sodium channel.
# We can trim the sodium current trace to that time-period and produce a plot.

trace_subset = trace[(trace['t'] > 100) & (trace['t'] < 120)]

plt.figure()
plt.plot('t', 'iNa', data=trace_subset)
plt.ylabel('Sodium Current (pA/pF)')
plt.xlabel('Time (ms)')
