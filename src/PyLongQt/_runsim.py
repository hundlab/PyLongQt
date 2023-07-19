#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Aug 23 2020

@author: grat05
"""

from time import sleep

from ._PyLongQt import RunSim

def printProgressLoop(self, noTime:bool=False):
    """
    Monitor the progress of the simulation and display a progressbar as well as
    the time taken
    :noTime: Do not display the time taken
    """
    text = '\r[{}{}] {prog}%'
    if not noTime:
        import datetime
        t0 = datetime.datetime.now()
        test += ' elsapsed {deltat}'
    taken_char = '\u2588'
    left_char = '-'
    term_len= 60
    while self.finished() == False:
        prog = self.progressPercent()
        n_taken = int(prog/100 * term_len)
        n_left = term_len - n_taken
        taken = taken_char*n_taken
        left = left_char*n_left
        fmt = dict(prog=round(prog, 3))
        if not noTime:
            t1 = datetime.datetime.now()
            deltat = t1 - t0
            fmt['deltat'] = str(deltat)
        print(text.format(taken, left, **fmt), end='')
        sleep(0.5)

RunSim.printProgressLoop = printProgressLoop

