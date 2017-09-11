#undef slots
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#define slots Q_SLOTS
#include <memory>

namespace py = pybind11;

#include "cellutils.h"
#include "cell/cell_kernel.h"
#include "cell/cell.h"
#include "cell/kurata08.h"
#include "cell/atrial.h"
#include "cell/gpbatrialSE.h"
#include "cell/ksan.h"
#include "cell/OHaraRudyEpi.h"
#include "cell/tnnp04.h"
#include "cell/br04.h"
#include "cell/gpbatrial.h"
#include "cell/gpbatrialWT.h"
#include "cell/hrd09.h"
#include "cell/kurata08.h"
#include "cell/OHaraRudy.h"
#include "cell/gpbatrialRyr.h"
#include "cell/gpbhuman.h"
#include "cell/inexcitablecell.h"
#include "cell/OHaraRudyEndo.h"
#include "cell/OHaraRudyM.h"
#include "cell/gridCell.h"
#include "protocol/protocol.h"
#include "protocol/currentClampProtocol.h"
#include "protocol/voltageClampProtocol.h"
#include "protocol/gridProtocol.h"
#include "structure/node.h"
#include "structure/fiber.h"
#include "structure/grid.h"
#include "cellpvars.h"
#include "measure.h"
#include "settingsIO.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

auto measMap = [] (string name) {
        auto c = make_shared<InexcitableCell>();
        auto meas = new MeasureManager(c);
        map<string,set<string>> measMap;
        for(auto var: meas->varsMeas) {
            measMap[var.first] = meas->varMeasCreator.at(var.second)({})->variables();
        }
        set<string> defaultMeas = Measure().variables();
        delete meas;
        return measMap.count(name) > 0? measMap[name]: defaultMeas;
    };

PYBIND11_MODULE(PyLongQt, m) {
    m.doc() = "python bindings for LongQt's cell models and protocols";
    //celluitls.h
    m.attr("cellMap") = py::cast(CellUtils::cellMap, py::return_value_policy::take_ownership);
    m.attr("protoMap") = py::cast(CellUtils::protoMap, py::return_value_policy::take_ownership);
    m.def("measMap",measMap, "Get the available measure variables for a given cell variable", py::arg("name"));
    py::enum_<CellUtils::Side>(m,"Side","Numbering of sides in 2D grids")
        .value("top", CellUtils::top)
        .value("bottom", CellUtils::bottom)
        .value("right", CellUtils::right)
        .value("left", CellUtils::left)
        .export_values();
    py::module m_Cells = m.def_submodule("Cells", "Contains all the Cell classes. For easy use cellMap provides constructors based on Cell.type.");
    py::module m_Protocols = m.def_submodule("Protocols", "Contains all the Protocol classes. For easy use protoMap provides constructors based on Protocol.type.");
    py::module m_Structures = m.def_submodule("Structures", "Structures used by GridProtocol for 1 & 2D simulations.");
    py::module m_Misc = m.def_submodule("Misc", "Measure, Pvars, helper classes");

    py::class_<CellKernel, std::shared_ptr<CellKernel>>(m_Cells, "CellKernel", "Base class of all cells")
        .def("clone",&CellKernel::clone)
        .def("reset",&CellKernel::reset)
        .def("updateV", &CellKernel::updateV, "Update voltages")
        .def("setV", &CellKernel::setV, "Set total voltage",py::arg("voltage"))
        .def("updateCurr", &CellKernel::updateCurr, "Update currents")
        .def("updateConc", &CellKernel::updateConc, "Update concentrations")
        .def("tstep", &CellKernel::tstep, "Increment the time used by the cell", py::arg("stimulus time"))
        .def("externalStim", &CellKernel::externalStim,"Stimulate the cell",py::arg("stimulus"))
        .def_readwrite("vOld", &CellKernel::vOld)
        .def_readwrite("vNew", &CellKernel::vNew)
        .def_readwrite("t", &CellKernel::t)
        .def_readwrite("dt", &CellKernel::dt)
        .def_readwrite("iNat", &CellKernel::iNat)
        .def_readwrite("iKt", &CellKernel::iKt)
        .def_readwrite("iCat", &CellKernel::iCat)
        .def_readwrite("iTot", &CellKernel::iTot)
        .def_readwrite("iTotold", &CellKernel::iTotold)
        .def_readwrite("Cm", &CellKernel::Cm)
        .def_readwrite("Vcell", &CellKernel::Vcell)
        .def_readwrite("Vmyo", &CellKernel::Vmyo)
        .def_readwrite("AGeo", &CellKernel::AGeo)
        .def_readwrite("ACap", &CellKernel::ACap)
        .def_readwrite("Rcg", &CellKernel::Rcg)
        .def_readwrite("cellRadius", &CellKernel::cellRadius)
        .def_readwrite("cellLength", &CellKernel::cellLength)
        .def_readwrite("dVdt", &CellKernel::dVdt)
        .def_readwrite("dVdtmax", &CellKernel::dVdtmax)
        .def_readwrite("Rmyo", &CellKernel::Rmyo)
        .def_readwrite("dtmin", &CellKernel::dtmin)
        .def_readwrite("dtmed", &CellKernel::dtmed)
        .def_readwrite("dtmax", &CellKernel::dtmax)
        .def_readwrite("dvcut", &CellKernel::dvcut)
        .def_readwrite("apTime", &CellKernel::apTime)
        .def_readwrite("RGAS", &CellKernel::RGAS)
        .def_readwrite("TEMP", &CellKernel::TEMP)
        .def_readwrite("FDAY", &CellKernel::FDAY)
//        .def_readonly("vars", &CellKernel::vars)
//        .def_readonly("pars", &CellKernel::pars)
        .def("getVar",&CellKernel::var,py::arg("name"))
        .def("setVar",&CellKernel::setVar,py::arg("name"),py::arg("value"))
        .def("getPar",&CellKernel::par,py::arg("name"))
        .def("setPar",&CellKernel::setPar,py::arg("name"),py::arg("value"))
        .def_property_readonly("variables",&CellKernel::getVariables)
        .def_property_readonly("constants",&CellKernel::getConstants)
        .def_property_readonly("type",&CellKernel::type);

        py::class_<Cell, std::shared_ptr<Cell>, CellKernel>(m_Cells, "Cell","Base class for cells. Provides selections of variables & constants to be written during the simulation")
//            .def(py::init<const std::string &>())
            .def_property("constantSelection", &Cell::getConstantSelection,&Cell::setConstantSelection)
            .def_property("variableSelection", &Cell::getVariableSelection,&Cell::setVariableSelection)
            .def("__repr__",[](Cell& c){return "<Cell Type='"+string(c.type())+"'>";});

        py::class_<ControlSa, std::shared_ptr<ControlSa>, Cell>(m_Cells, "ControlSa")
            .def(py::init<>());
        py::class_<OHaraRudy, std::shared_ptr<OHaraRudy>, Cell>(m_Cells, "OHaraRudy")
            .def(py::init<>());
        py::class_<OHaraRudyEndo, std::shared_ptr<OHaraRudyEndo>,OHaraRudy>(m_Cells, "OHaraRudyEndo")
            .def(py::init<>());
        py::class_<OHaraRudyEpi, std::shared_ptr<OHaraRudyEpi>,OHaraRudy>(m_Cells, "OHaraRudyEpi")
            .def(py::init<>());
        py::class_<OHaraRudyM, std::shared_ptr<OHaraRudyM>,OHaraRudy>(m_Cells, "OHaraRudyM")
            .def(py::init<>());
        py::class_<Courtemanche98, std::shared_ptr<Courtemanche98>, Cell>(m_Cells, "Courtemanche98")
            .def(py::init<>());
        py::class_<Br04, std::shared_ptr<Br04>, Cell>(m_Cells, "Br04")
            .def(py::init<>());
        py::class_<GpbAtrial, std::shared_ptr<GpbAtrial>, Cell>(m_Cells, "GpbAtrial")
            .def(py::init<>());
        py::class_<GpbVent, std::shared_ptr<GpbVent>, Cell>(m_Cells, "GpbVent")
            .def(py::init<>());
        py::class_<HRD09Control, std::shared_ptr<HRD09Control>, Cell>(m_Cells, "HRD09Control")
            .def(py::init<>());
        py::class_<HRD09BorderZone, std::shared_ptr<HRD09BorderZone>, HRD09Control>(m_Cells, "HRD09BorderZone")
            .def(py::init<>());
        py::class_<InexcitableCell, std::shared_ptr<InexcitableCell>, Cell>(m_Cells, "InexcitableCell")
            .def(py::init<>());
        py::class_<Ksan, std::shared_ptr<Ksan>, Cell>(m_Cells, "Ksan")
            .def(py::init<>());
        py::class_<TNNP04Control, std::shared_ptr<TNNP04Control>, Cell>(m_Cells, "TNNP04Control")
            .def(py::init<>());
        py::class_<GridCell, std::shared_ptr<GridCell>, Cell>(m_Cells, "GridCell")
            .def(py::init<>())
            .def_property_readonly("grid", &GridCell::getGrid);

        py::class_<GetSetRef>(m_Protocols, "GetSetRef", "Accessor for properties in protocol")
            .def(py::init<>())
            .def_property("val", [](const GetSetRef& r) {return r.get();},[](const GetSetRef& r,string arg) {r.set(arg);})
            .def("__repr__", [](const GetSetRef& r)
                {return "<GetSet Type="+r.type+" Val='"+r.get()+"'>";})
            .def_readonly("type",&GetSetRef::type);

        py::class_<Protocol,std::shared_ptr<Protocol>>(m_Protocols, "Protocol", "Protocols determine what happens in the simulation. These classes contain configurations for the simulations.")
            .def("clone",&Protocol::clone)
            .def("readInCellState",&Protocol::readInCellState,"Read in cell from cellstatedir")
            .def("writeOutCellState",&Protocol::writeOutCellState,"Write out final state of the cell to datadir")
            .def("runSim", &Protocol::runSim, "Run all the trials consecutively")
            .def_property("trial",(unsigned int(Protocol::*)(void)const)&Protocol::trial,(void(Protocol::*)(unsigned int))&Protocol::trial)
            .def("runTrial", &Protocol::runTrial,"Run the current trial")
//            .def("setupTrial", &Protocol::setupTrial)
            .def_property("cell",(shared_ptr<Cell>(Protocol::*)(void)const)&Protocol::cell,(void(Protocol::*)(shared_ptr<Cell>))&Protocol::cell,py::arg("cell"))
            .def("setCellByName",(bool(Protocol::*)(const string&))&Protocol::cell)
            .def("cellOptions",&Protocol::cellOptions,"possible cells that can be set with .cell")
            .def_property_readonly("pvars",&Protocol::pvars,py::return_value_policy::reference_internal,"The property settings. Used to modify the cells constants at the beginning of each simulation.")
            .def_property_readonly("measureMgr",&Protocol::measureMgr,py::return_value_policy::reference_internal,"The measurement settings. Tracks min,peak,etc of a cell variable")
            .def_readonly("type",&Protocol::type)
//            .def_readwrite("__dict__",&Protocol::pars)
            .def_property("cellStateDir", [](Protocol& p){return p.pars["cellStateDir"].get();},[](Protocol& p,string val){p.pars["datadir"].set(val);})
            .def_readwrite("cellStateFile",&Protocol::cellStateFile)
            .def_property("datadir",[](Protocol& p){return p.pars["datadir"].get();},[](Protocol& p,string val){p.pars["datadir"].set(val);})
            .def_readwrite("meastime",&Protocol::meastime)
            .def_readwrite("numtrials",&Protocol::numtrials)
            .def_readwrite("readCellState",&Protocol::readCellState)
            .def_readwrite("simvarfile",&Protocol::simvarfile)
            .def_readwrite("tMax",&Protocol::tMax)
            .def_readwrite("writeCellState",&Protocol::writeCellState)
            .def_readwrite("writeint",&Protocol::writeint)
            .def_readwrite("writetime",&Protocol::writetime)
            .def("setDataDir",&Protocol::setDataDir)
            .def("mkDirs",&Protocol::mkDirs)
            .def_readonly("pars",&Protocol::pars);

        py::class_<CurrentClamp, Protocol, std::shared_ptr<CurrentClamp>>(m_Protocols, "CurrentClamp")
            .def(py::init<>())
            .def_readwrite("bcl",&CurrentClamp::bcl)
            .def_readwrite("paceflag",&CurrentClamp::paceflag)
            .def_readwrite("numstims",&CurrentClamp::numstims)
            .def_readwrite("stimdur",&CurrentClamp::stimdur)
            .def_readwrite("stimval",&CurrentClamp::stimval)
            .def_readwrite("stimt",&CurrentClamp::stimt);
        py::class_<VoltageClamp, Protocol, std::shared_ptr<VoltageClamp>>(m_Protocols, "VoltagesClamp")
            .def(py::init<>())
            .def_readwrite("t1",&VoltageClamp::t1)
            .def_readwrite("t2",&VoltageClamp::t2)
            .def_readwrite("t3",&VoltageClamp::t3)
            .def_readwrite("t4",&VoltageClamp::t4)
            .def_readwrite("t5",&VoltageClamp::t5)
            .def_readwrite("v1",&VoltageClamp::v1)
            .def_readwrite("v2",&VoltageClamp::v2)
            .def_readwrite("v3",&VoltageClamp::v3)
            .def_readwrite("v4",&VoltageClamp::v4)
            .def_readwrite("v5",&VoltageClamp::v5);
        py::class_<GridProtocol, CurrentClamp,std::shared_ptr<GridProtocol>>(m_Protocols, "GridProtocol")
            .def(py::init<>())
            .def_property_readonly("grid", &GridProtocol::getGrid)
            .def_property_readonly("stimNodes", &GridProtocol::getStimNodes, "nodes to stimulate")
            .def_property("stim2", &GridProtocol::getStim2, &GridProtocol::setStim2,"enable a second stimulus")
            .def_readwrite("stimNodes",&GridProtocol::stimNodes)
            .def_readwrite("stimNodes2",&GridProtocol::stimNodes2)
            .def_readwrite("stimval2",&GridProtocol::stimval2)
            .def_readwrite("stimdur2",&GridProtocol::stimdur2)
            .def_readwrite("bcl2",&GridProtocol::bcl2)
            .def_readwrite("stimt2",&GridProtocol::stimt2);

        py::class_<Node, std::shared_ptr<Node>>(m_Structures, "Node", "Finest unit in a multi-dimensional simulation, contains a cell")
            .def(py::init<>())
            .def("setCondConst", &Node::setCondConst, "set the conductivity values", py::arg("X"),py::arg("dx"),py::arg("s"),py::arg("perc")=true,py::arg("val")=1)
            .def_readonly("cell", &Node::cell)
//            .def_readwrite("rd",&Node::rd)
            .def_readwrite("condConst", &Node::condConst)
            .def_readwrite("np",&Node::np,"number of cells represented by the node")
            .def_readwrite("type",&Node::nodeType)
            .def("__repr__",[](Node& n){return "<Node "+string(py::repr(py::cast(n.cell)))+">";});
        py::class_<Fiber>(m_Structures, "Fiber", "A 1D line of nodes")
            .def(py::init<int>())
            .def("updateVm",&Fiber::updateVm,"Update voltages for all nodes")
//            .def_readwrite("nodes",&Fiber::nodes)
            .def_readwrite("B",&Fiber::B)
            .def("__len__",&Fiber::size)
            .def("__getitem__",&Fiber::operator[])
            .def("__iter__", [](const Fiber& f)
                { return py::make_iterator(f.begin(), f.end()); },
                py::keep_alive<0, 1>())
            .def("__repr__", [](Fiber& f){
                string repr = "Fiber([";
                bool first = true;
                for(auto& node: f.nodes) {
                    if(!first) repr+=", ";
                    repr+=string(py::repr(py::cast(node)));
                    first=false;
                }
                repr += "])";
                return repr;
            });
        py::class_<CellInfo>(m_Structures, "CellInfo", "Contains all the info needed to insert a cell into a Grid")
            .def(py::init<int,int,double,double,int,shared_ptr<Cell>,array<double,4>,bool>(),py::arg("X")=-1,py::arg("Y")=-1,py::arg("dx")=0.01,py::arg("dy")=0.01,py::arg("np")=1,py::arg("cell")=nullptr,py::arg("c")=array<double,4>({NAN,NAN,NAN,NAN}),py::arg("c_perc")=false)
            .def_readwrite("X",&CellInfo::X)
            .def_readwrite("Y",&CellInfo::Y)
            .def_readwrite("dx",&CellInfo::dx)
            .def_readwrite("dy",&CellInfo::dy)
            .def_readwrite("np",&CellInfo::np)
            .def_readwrite("c_perc",&CellInfo::c_perc)
            .def_readwrite("cell",&CellInfo::cell)
            .def_readwrite("c",&CellInfo::c)
            .def("__repr__", [](const CellInfo& r)
                {return "<CellInfo X="+to_string(r.X)+", Y="+to_string(r.Y)+
                (r.cell? ", cell='"+string(r.cell->type())+"'>": "");});

        py::class_<Grid>(m_Structures, "Grid")
            .def(py::init<>())
            .def("addRow",&Grid::addRow,py::arg("pos"))
            .def("addRows",&Grid::addRows,py::arg("num"),py::arg("position")=0)
            .def("addColumn",&Grid::addColumn,py::arg("pos"))
            .def("addColumns",&Grid::addColumns,py::arg("num"),py::arg("position")=0)
            .def("removeRow",&Grid::removeRow,py::arg("pos"))
            .def("removeRows",&Grid::removeRows,py::arg("num"),py::arg("pos"))
            .def("removeColumn",&Grid::removeColumn,py::arg("pos"))
            .def("removeColumns",&Grid::removeColumns,py::arg("num"),py::arg("pos"))
            .def("setCells",(void(Grid::*)(list<CellInfo>&))&Grid::setCellTypes,"set cells in grid",py::arg("cells"))
            .def("setCell",(void(Grid::*)(const CellInfo&))&Grid::setCellTypes,py::arg("singleCell"))
            .def_property_readonly("shape",[](Grid& g){return make_tuple(g.rowCount(),g.columnCount());})
            .def("rowCount",&Grid::rowCount)
            .def("columnCount",&Grid::columnCount)
            .def("findNode",&Grid::findNode,"find a nodes position")
            .def("reset",&Grid::reset)
            .def("updateB",&Grid::updateB,"use node conductivity values to update fiber conductivity list")
            .def("__len__",&Grid::columnCount)
            .def("__iter__", [](const Grid& g)
                { return py::make_iterator(g.begin(), g.end()); },
                py::keep_alive<0, 1>())
            .def("__getitem__",(shared_ptr<Node>(Grid::*)(const int, const int))&Grid::operator())
            .def("__getitem__",[](Grid& g,list<pair<int,int>> l){
                list<shared_ptr<Node>> resList;
                for(auto& item: l) {
                    resList.push_back(g(item));
                }
                return resList;
            })
            .def("__repr__",[](Grid& g){
                 string repr = "Grid([";
                 bool first = true;
                 for(Fiber& f: g.fiber) {
                    if(!first) repr+=", ";
                    repr+=string(py::repr(py::cast(f)));
                    first = false;
                 }
                 repr+="])";
                 return repr;
            });

        auto cellPvarsREPR = [](CellPvars& p){
                string out = "{";
                bool first = true;
                for(auto& par: p) {
                    if(!first) out += ", ";
                    out += "'"+par.first+"': "+string(py::repr(py::cast(par.second)));
                    first = false;
                }
                out += "}";
                return out;
        };
        py::class_<CellPvars> cellpvars(m_Misc, "CellPvars","Sets cell constant values");
        cellpvars.def("setIonChanParams",&CellPvars::setIonChanParams,"apply ion channel parameters")
            .def("calcIonChanParams",&CellPvars::calcIonChanParams,"calculate the ion channel parameters")
            .def("__setitem__",&CellPvars::insert,"insert new rule",py::arg("name"),py::arg("parameter"))
            .def("__delitem__",&CellPvars::erase)
            .def("clear",&CellPvars::clear,"remove all rules")
            .def("__getitem__",&CellPvars::at)
            .def("__iter__", [](const CellPvars &pvars)
                { return py::make_iterator(pvars.begin(), pvars.end()); },
                py::keep_alive<0, 1>())
            .def("__len__",&CellPvars::size)
            .def("__repr__",cellPvarsREPR);
        py::enum_<CellPvars::Distribution>(cellpvars, "Distribution")
            .value("none", CellPvars::Distribution::none)
            .value("normal", CellPvars::Distribution::normal)
            .value("lognormal", CellPvars::Distribution::lognormal)
            .export_values();
        auto IonChanParamREPR = [](CellPvars::IonChanParam& p){
                auto s = QString(CellUtils::trim(p.IonChanParam::str("")).c_str());
                auto split = s.split(QRegExp("\\t"), QString::SkipEmptyParts);
                return (split[0]+" ("+split[1]+" "+split[2]+")").toStdString();
        };
        py::class_<CellPvars::IonChanParam>(cellpvars, "IonChanParam", "A rule for a cell constant")
            .def(py::init<CellPvars::Distribution,double,double>(),py::arg("dist")=CellPvars::Distribution::none,py::arg("val1")=0,py::arg("val2")=0)
            .def_readwrite("dist",&CellPvars::IonChanParam::dist)
            .def_property("val0",[](const CellPvars::IonChanParam& param) {
                return param.val[0];
            },
            [](CellPvars::IonChanParam& param, double val) {
                param.val[0] = val;
            },"Depends on distribution\n none: starting value\n normal & lognormal: mean")
            .def_property("val1",[](const CellPvars::IonChanParam& param) {
                return param.val[1];
            },
            [](CellPvars::IonChanParam& param, double val) {
                param.val[1] = val;
            },"Depends on distribution\nnone: increment amount\nnormal & lognormal: standard deviation")
            .def("__repr__",IonChanParamREPR);

        py::class_<PvarsCurrentClamp, CellPvars> pvarsCurr(m_Misc, "PvarsCurrentClamp");
        pvarsCurr.def(py::init<Protocol*>())
            .def("protocol",&PvarsCurrentClamp::protocol)
            .def("__repr__",[cellPvarsREPR](PvarsCurrentClamp& c){
                return "PvarsCurrentClamp("+cellPvarsREPR(c)+")";
            });
        py::class_<PvarsCurrentClamp::TIonChanParam, CellPvars::IonChanParam>(
            pvarsCurr, "TIonChanParam")
            .def(py::init<>())
            .def_readwrite("trials",&PvarsCurrentClamp::TIonChanParam::trials)
            .def("__repr__",[IonChanParamREPR](PvarsCurrentClamp::TIonChanParam& p){return "<TIonChanParam: "+IonChanParamREPR(p)+">";});

        py::class_<PvarsVoltageClamp, CellPvars> pvarsVolt(m_Misc, "PvarsVoltageClamp");
        pvarsVolt.def(py::init<Protocol*>())
            .def("protocol",&PvarsVoltageClamp::protocol)
            .def("__repr__",[cellPvarsREPR](PvarsVoltageClamp& c){
                return "PvarsVoltageClamp("+cellPvarsREPR(c)+")";
            });
        py::class_<PvarsVoltageClamp::SIonChanParam, CellPvars::IonChanParam>(
            pvarsVolt, "SIonChanParam")
            .def(py::init<>())
            .def_readwrite("paramVal",&PvarsVoltageClamp::SIonChanParam::paramVal)
            .def("__repr__",[IonChanParamREPR](PvarsVoltageClamp::SIonChanParam& p){return "<SIonChanParam: "+IonChanParamREPR(p)+">";});

        py::class_<PvarsGrid, CellPvars> pvarsGrid(m_Misc, "PvarsGrid");
        pvarsGrid.def(py::init<Grid*>())
            .def("setGrid",&PvarsGrid::setGrid)
            .def("setMaxDistAndVal",&PvarsGrid::setMaxDistAndVal,"Set the maximum distance and the maximum value for a rule",py::arg("name"),py::arg("maxDist"),py::arg("maxVal"))
            .def("setStartCells",&PvarsGrid::setStartCells,"Set the cell locations that the rule will start from",py::arg("name"),py::arg("startCells"))
            .def("__repr__",[cellPvarsREPR](PvarsGrid& c){
                return "PvarsGrid("+cellPvarsREPR(c)+")";
            });
        py::class_<PvarsGrid::MIonChanParam, CellPvars::IonChanParam>(
            pvarsGrid, "MIonChanParam")
            .def(py::init<>())
            .def_readwrite("maxDist",&PvarsGrid::MIonChanParam::maxDist)
            .def_readwrite("maxVal",&PvarsGrid::MIonChanParam::maxVal)
            .def_readwrite("startCells",&PvarsGrid::MIonChanParam::startCells)
            .def_readwrite("cells",&PvarsGrid::MIonChanParam::cells)
            .def("__repr__",[IonChanParamREPR](PvarsGrid::MIonChanParam& p){return "<MIonChanParam: "+IonChanParamREPR(p)+">";});

        py::class_<Measure>(m_Misc, "Measure","Class that measures an individual variable from a cell")
            .def(py::init<>())
            .def("measure",&Measure::measure)
            .def("reset",&Measure::reset)
            .def_property_readonly("variables",&Measure::variables)
            .def_property_readonly("variablesMap",&Measure::variablesMap)
            .def_property("selection",(set<string>(Measure::*)(void))&Measure::selection,(void(Measure::*)(set<string>))&Measure::selection)
            .def("nameString",&Measure::getNameString)
            .def("valueString",&Measure::getValueString);
        py::class_<MeasureWave,Measure>(m_Misc, "MeasureWave")
            .def(py::init<>())
            .def_property("percrepol",(double(MeasureWave::*)(void)const)&MeasureWave::percrepol,(void(MeasureWave::*)(double))&MeasureWave::percrepol);

        py::class_<MeasureManager>(m_Misc, "MeasureManager","Manages measuring variables from a cell during a simulation. Can record per cycle measurements of min,peak,etc")
            .def(py::init<shared_ptr<Cell>>())
            .def_property("cell", (shared_ptr<Cell>(MeasureManager::*)(void))&MeasureManager::cell,(void(MeasureManager::*)(shared_ptr<Cell>))&MeasureManager::cell)
            .def_property("selection",(map<string,set<string>>(MeasureManager::*)(void))&MeasureManager::selection,(void(MeasureManager::*)(map<string,set<string>>))&MeasureManager::selection,"variables to be measured and their measured properties")
            .def_property("percrepol",(double(MeasureManager::*)(void))&MeasureManager::percrepol,(void(MeasureManager::*)(double))&MeasureManager::percrepol,"the percent repolarization to write out measurements")
            .def("createMeasure",&MeasureManager::getMeasure,"Create a Measure",py::arg("varname"),py::arg("selection"))
            .def("addMeasure",&MeasureManager::addMeasure,"Add a new cell variable to be measured",py::arg("varname"),py::arg("selection")=set<string>())
            .def("setupMeasures",&MeasureManager::setupMeasures,"Get measures ready for simulation",py::arg("filename"))
            .def("measure",&MeasureManager::measure,"measure the variables",py::arg("time"))
            .def("clear",&MeasureManager::clear)
            .def("resetMeasures",&MeasureManager::resetMeasures,"reset measures that are invalid after percrepol is reached")
            .def_readonly("varsMeas",&MeasureManager::varsMeas)
            .def_readonly("varMeasCreator",&MeasureManager::varMeasCreator);
        py::class_<GridMeasureManager,MeasureManager>(m_Misc, "GridMeasureManager")
            .def(py::init<shared_ptr<GridCell>>())
            .def_property("dataNodes",(set<pair<int,int>>(GridMeasureManager::*)(void))&GridMeasureManager::dataNodes,(void(GridMeasureManager::*)(set<pair<int,int>>))&GridMeasureManager::dataNodes)
            .def("setGridCell",(void(GridMeasureManager::*)(GridCell* cell))&GridMeasureManager::cell);

        py::class_<SettingsIO>(m_Misc, "SettingsIO")
            .def("getInstance",&SettingsIO::getInstance)
//            .def("writedvars",&SettingsIO::writedvars)
//            .def("readdvars",&SettingsIO::readdvars)
            .def_readonly("lastProto",&SettingsIO::lastProto)
            .def_readwrite("allowProtoChange",&SettingsIO::allowProtoChange)
            .def("readSettings", [](SettingsIO& s,shared_ptr<Protocol> proto,char* filename){s.readSettings(proto,filename);})
            .def("writeSettings", [](SettingsIO& s,shared_ptr<Protocol> proto,char* filename){s.writeSettings(proto,filename);});
}
// {
//    m.doc() = "";
    // optional module docstring
//    m.def("cellMap", &bain, "A function which returns a random number!");
//    m.def("trim", &CellUtils::trim, "trim whitespace from beginning and end of a string", py::arg("string"));
//}
