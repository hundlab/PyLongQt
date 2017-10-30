#include "protocol/protocol.h"
#include "protocol/currentClampProtocol.h"
#include "protocol/voltageClampProtocol.h"
#include "protocol/gridProtocol.h"
#include "settingsIO.h"
#include "pylongqt.h"

void init_protocols(py::module &m) {
    py::module m_Protocols = m.def_submodule("Protocols", "Contains all the Protocol classes. For easy use protoMap provides constructors based on Protocol.type.");

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
        .def_property("cell",(shared_ptr<Cell>(Protocol::*)(void)const)&Protocol::cell,(void(Protocol::*)(shared_ptr<Cell>))&Protocol::cell)
        .def("setCellByName",(bool(Protocol::*)(const string&))&Protocol::cell)
        .def("cellOptions",&Protocol::cellOptions,"possible cells that can be set with .cell")
        .def_property_readonly("pvars",&Protocol::pvars,py::return_value_policy::reference_internal,"The property settings. Used to modify the cells constants at the beginning of each simulation.")
        .def_property_readonly("measureMgr",&Protocol::measureMgr,py::return_value_policy::reference_internal,"The measurement settings. Tracks min,peak,etc of a cell variable")
        .def_property_readonly("type",&Protocol::type)
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
        .def("setDataDir",&Protocol::setDataDir, py::arg("str")="")
        .def("mkDirs",&Protocol::mkDirs)
        .def_readwrite("numruns",&Protocol::numruns)
        .def_readwrite("runEvery",&Protocol::runEvery)
        .def_readwrite("firstRun",&Protocol::firstRun)
        .def_property("runBefore",&Protocol::getRunBefore,&Protocol::setRunBefore)
        .def_property("runDuring",&Protocol::getRunDuring,&Protocol::setRunDuring)
        .def_property("runAfter",&Protocol::getRunAfter,&Protocol::setRunAfter)
        .def_readonly("pars",&Protocol::pars);

    py::class_<CurrentClamp, Protocol, std::shared_ptr<CurrentClamp>>(m_Protocols, "CurrentClamp")
        .def(py::init<>())
        .def(py::pickle(
        [] (shared_ptr<Protocol> p) {
            QString str;
            SettingsIO::getInstance()->writeSettingsStr(p,&str);
            return str.toStdString();
        },
        [] (const string& s) {
            QString str(s.c_str());
            auto p = CellUtils::protoMap.at("Current Clamp Protocol")();
            auto set = SettingsIO::getInstance();
            set->allowProtoChange = false;
            set->readSettingsStr(p, str);
            set->allowProtoChange = true;
            return *dynamic_pointer_cast<CurrentClamp>(p);
        }
        ))
        .def_readwrite("bcl",&CurrentClamp::bcl)
        .def_readwrite("paceflag",&CurrentClamp::paceflag)
        .def_readwrite("numstims",&CurrentClamp::numstims)
        .def_readwrite("stimdur",&CurrentClamp::stimdur)
        .def_readwrite("stimval",&CurrentClamp::stimval)
        .def_readwrite("stimt",&CurrentClamp::stimt);
    py::class_<VoltageClamp, Protocol, std::shared_ptr<VoltageClamp>>(m_Protocols, "VoltageClamp")
        .def(py::init<>())
        .def(py::pickle(
        [] (shared_ptr<Protocol> p) {
            QString str;
            SettingsIO::getInstance()->writeSettingsStr(p,&str);
            return str.toStdString();
        },
        [] (const string& s) {
            QString str(s.c_str());
            auto p = CellUtils::protoMap.at("Voltage Clamp Protocol")();
            auto set = SettingsIO::getInstance();
            set->allowProtoChange = false;
            set->readSettingsStr(p, str);
            set->allowProtoChange = true;
            return *dynamic_pointer_cast<VoltageClamp>(p);
        }
        ))
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
        .def(py::pickle(
        [] (shared_ptr<Protocol> p) {
            QString str;
            SettingsIO::getInstance()->writeSettingsStr(p,&str);
            return str.toStdString();
        },
        [] (const string& s) {
            QString str(s.c_str());
            auto p = CellUtils::protoMap.at("Grid Protocol")();
            auto set = SettingsIO::getInstance();
            set->allowProtoChange = false;
            set->readSettingsStr(p, str);
            set->allowProtoChange = true;
            return *dynamic_pointer_cast<GridProtocol>(p);
        }
        ))
        .def_property_readonly("grid", &GridProtocol::getGrid)
        .def_property_readonly("stimNodes", &GridProtocol::getStimNodes, "nodes to stimulate")
        .def_property("stim2", &GridProtocol::getStim2, &GridProtocol::setStim2,"enable a second stimulus")
        .def_readwrite("stimNodes",&GridProtocol::stimNodes)
        .def_readwrite("stimNodes2",&GridProtocol::stimNodes2)
        .def_readwrite("stimval2",&GridProtocol::stimval2)
        .def_readwrite("stimdur2",&GridProtocol::stimdur2)
        .def_readwrite("bcl2",&GridProtocol::bcl2)
        .def_readwrite("stimt2",&GridProtocol::stimt2);
}
