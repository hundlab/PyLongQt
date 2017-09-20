from hypothesis import given
import hypothesis.strategies as st
import PyLongQt as plqt


#@given(st.text())
#def test_decode_inverts_encode(s):
#    assert decode(encode(s)) == s

#cellkernel

#cell
#types?

#protocol

#Node
#Fiber
#Grid
#CellPvars
 #PvarsCurrentClamp
@given(st.text(),st.floats(allow_nan=False,allow_infinity=False),st.floats(allow_nan=False,allow_infinity=False),st.integers(min_value=0,max_value=100))
def test_PvarsCurrentClamp_calcIonChanParams(name,val1,val2,numtrials):
    p = plqt.Protocols.CurrentClamp()
    c = plqt.Misc.PvarsCurrentClamp(p)
    c[name] = c.IonChanParam(c.none,val1,val2)
    p.numtrials = numtrials
    c.calcIonChanParams()
    assert len(c[name].trials) == numtrials
    for i,val in enumerate(c[name].trials):
        assert val == val1+i*val2
@given(st.lists(st.text(),unique=True),st.floats(allow_nan=False),st.floats(allow_nan=False))
def test_PvarsCurrentClamp_insert(names,val1,val2):
    c = plqt.Misc.PvarsCurrentClamp(plqt.Protocols.CurrentClamp())
    i = 1
    for name in names:
        c[name] = c.IonChanParam(c.normal,val1,val2)
        assert len(c) == i
        assert c[name].val0 == val1
        assert c[name].val1 == val2
        assert type(c[name]) == type(c.TIonChanParam())
        i += 1
@given(st.sets(st.text()),st.sets(st.text()),st.floats(allow_nan=False),st.floats(allow_nan=False))
def test_PvarsCurrentClamp_delete(names,deletes,val1,val2):
    c = plqt.Misc.PvarsCurrentClamp(plqt.Protocols.CurrentClamp())
    i = 1
    for name in names:
        c[name] = c.IonChanParam(c.normal,val1,val2)
        i += 1
    i -= 1
    deletes = deletes.union(names)
    for name in sorted(deletes):
        del c[name]
        if name in names:
            i -= 1
            assert len(c) == i
@given(st.lists(st.text(),unique=True),st.floats(allow_nan=False),st.floats(allow_nan=False))
def test_PvarsCurrentClamp_clear(names,val1,val2):
    c = plqt.Misc.PvarsCurrentClamp(plqt.Protocols.CurrentClamp())
    i = 1
    for name in names:
        c[name] = c.IonChanParam(c.normal,val1,val2)
        i += 1
    c.clear()
    assert len(c) == 0
 #PvarsVoltageClamp
@given(st.text(),st.floats(allow_nan=False,allow_infinity=False))
def test_PvarsVoltageClamp_calcIonChanParams(name,val1):
    p = plqt.Protocols.VoltageClamp()
    c = plqt.Misc.PvarsVoltageClamp(p)
    c[name] = c.IonChanParam(c.none,val1,0)
    c.calcIonChanParams()
    assert c[name].val == val1
@given(st.lists(st.text(),unique=True),st.floats(allow_nan=False),st.floats(allow_nan=False))
def test_PvarsVoltageClamp_insert(names,val1,val2):
    c = plqt.Misc.PvarsVoltageClamp(plqt.Protocols.VoltageClamp())
    i = 1
    for name in names:
        c[name] = c.IonChanParam(c.normal,val1,val2)
        assert len(c) == i
        assert c[name].val0 == val1
        assert c[name].val1 == val2
        assert type(c[name]) == type(c.SIonChanParam())
        i += 1
@given(st.sets(st.text()),st.sets(st.text()),st.floats(allow_nan=False),st.floats(allow_nan=False))
def test_PvarsVoltageClamp_delete(names,deletes,val1,val2):
    c = plqt.Misc.PvarsVoltageClamp(plqt.Protocols.VoltageClamp())
    i = 1
    for name in names:
        c[name] = c.IonChanParam(c.normal,val1,val2)
        i += 1
    i -= 1
    deletes = deletes.union(names)
    for name in sorted(deletes):
        del c[name]
        if name in names:
            i -= 1
            assert len(c) == i
@given(st.lists(st.text(),unique=True),st.floats(allow_nan=False),st.floats(allow_nan=False))
def test_PvarsVoltageClamp_clear(names,val1,val2):
    c = plqt.Misc.PvarsVoltageClamp(plqt.Protocols.VoltageClamp())
    i = 1
    for name in names:
        c[name] = c.IonChanParam(c.normal,val1,val2)
        i += 1
    c.clear()
    assert len(c) == 0

 #PvarsGrid
@given(st.text(),st.floats(allow_nan=False,allow_infinity=False),st.integers(min_value=0,max_value=50),st.integers(min_value=0,max_value=50))
def test_PvarsGrid_calcIonChanParams(name,val1,nr,nc):
    p = plqt.Protocols.GridProtocol()
    c = p.pvars
    c[name] = c.IonChanParam(c.none,val1,0)
    p.grid.addRows(nr)
    p.grid.addColumns(nc)
    c.setStartCells(name,set([(0,0)]))
    c.setMaxDistAndVal(name,1000,val1+1)
    c.calcIonChanParams()
    for i,val in enumerate(c[name].cells.values()):
        assert val == val1
    assert len(c[name].cells) == nr*nc
@given(st.lists(st.text(),unique=True),st.floats(allow_nan=False),st.floats(allow_nan=False))
def test_PvarsGrid_insert(names,val1,val2):
    c = plqt.Misc.PvarsGrid(plqt.Structures.Grid())
    i = 1
    for name in names:
        c[name] = c.IonChanParam(c.normal,val1,val2)
        assert len(c) == i
        assert c[name].val0 == val1
        assert c[name].val1 == val2
        assert type(c[name]) == type(c.MIonChanParam())
        i += 1
@given(st.sets(st.text()),st.sets(st.text()),st.floats(allow_nan=False),st.floats(allow_nan=False))
def test_PvarsGrid_delete(names,deletes,val1,val2):
    c = plqt.Misc.PvarsGrid(plqt.Structures.Grid())
    i = 1
    for name in names:
        c[name] = c.IonChanParam(c.normal,val1,val2)
        i += 1
    i -= 1
    deletes = deletes.union(names)
    for name in sorted(deletes):
        del c[name]
        if name in names:
            i -= 1
            assert len(c) == i
@given(st.lists(st.text(),unique=True),st.floats(allow_nan=False),st.floats(allow_nan=False))
def test_PvarsGrid_clear(names,val1,val2):
    c = plqt.Misc.PvarsGrid(plqt.Structures.Grid())
    i = 1
    for name in names:
        c[name] = c.IonChanParam(c.normal,val1,val2)
        assert len(c) == i
        i += 1
    c.clear()
    assert len(c) == 0


#Measure
#WaveMeasure
#MeasureManager
#GridMeasureManager
#settingsIO?

