#include "pvarscell.h"
#include "measure.h"
#include "settingsIO.h"
#include "pvarscurrentclamp.h"
#include "pvarsvoltageclamp.h"
#include "pvarsgrid.h"
#include "pylongqt.h"
#include "measurewave.h"
#include "measuremanager.h"
#include "gridmeasuremanager.h"
#include "runsim.h"
using namespace LongQt;
using namespace std;

void init_misc(py::module &m) {
    py::module m_Misc = m.def_submodule("Misc", "Measure, Pvars, helper classes");

    auto pvarscell_REPR = [](PvarsCell& p){
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
    py::class_<PvarsCell> pvarscell(m_Misc, "_PvarsCell","Sets cell constant values");
    pvarscell.def("setIonChanParams",&PvarsCell::setIonChanParams,"apply ion channel parameters")
        .def("calcIonChanParams",&PvarsCell::calcIonChanParams,"calculate the ion channel parameters")
        .def("__setitem__",&PvarsCell::insert,"insert new rule",py::arg("name"),py::arg("parameter"))
        .def("__delitem__",&PvarsCell::erase)
        .def("clear",&PvarsCell::clear,"remove all rules")
        .def("__getitem__",&PvarsCell::at,py::return_value_policy::reference_internal)
        .def("__iter__", [](const PvarsCell &pvars)
            { return py::make_iterator(pvars.begin(), pvars.end(),py::return_value_policy::reference_internal); },
            py::keep_alive<0, 1>())
        .def("__len__",&PvarsCell::size)
        .def("__repr__",pvarscell_REPR);
    py::enum_<PvarsCell::Distribution>(pvarscell, "Distribution")
        .value("none", PvarsCell::Distribution::none)
        .value("normal", PvarsCell::Distribution::normal)
        .value("lognormal", PvarsCell::Distribution::lognormal)
        .export_values();
    auto IonChanParamREPR = [](PvarsCell::IonChanParam& p){
            auto s = QString(CellUtils::trim(p.IonChanParam::str("")).c_str());
            auto split = s.split(QRegExp("\\t"), QString::SkipEmptyParts);
            return (split[0]+" ("+split[1]+" "+split[2]+")").toStdString();
    };
    py::class_<PvarsCell::IonChanParam>(pvarscell, "IonChanParam", "A rule for a cell constant")
        .def(py::init<PvarsCell::Distribution,double,double>(),py::arg("dist")=PvarsCell::Distribution::none,py::arg("val1")=0,py::arg("val2")=0)
        .def_readwrite("dist",&PvarsCell::IonChanParam::dist)
        .def_property("val0",[](const PvarsCell::IonChanParam& param) {
            return param.val[0];
        },
        [](PvarsCell::IonChanParam& param, double val) {
            param.val[0] = val;
        },"Depends on distribution\n none: starting value\n normal & lognormal: mean")
        .def_property("val1",[](const PvarsCell::IonChanParam& param) {
            return param.val[1];
        },
        [](PvarsCell::IonChanParam& param, double val) {
            param.val[1] = val;
        },"Depends on distribution\nnone: increment amount\nnormal & lognormal: standard deviation")
        .def("__repr__",IonChanParamREPR);

    py::class_<PvarsCurrentClamp, PvarsCell> pvarsCurr(m_Misc, "_PvarsCurrentClamp");
    pvarsCurr.def(py::init<Protocol*>(),py::keep_alive<1,2>())
        .def("__repr__",[pvarscell_REPR](PvarsCurrentClamp& c){
            return "PvarsCurrentClamp("+pvarscell_REPR(c)+")";
        });
    py::class_<PvarsCurrentClamp::TIonChanParam, PvarsCell::IonChanParam>(
        pvarsCurr, "_TIonChanParam")
        .def(py::init<>())
        .def_readwrite("trials",&PvarsCurrentClamp::TIonChanParam::trials)
        .def("__repr__",[IonChanParamREPR](PvarsCurrentClamp::TIonChanParam& p){return "<TIonChanParam: "+IonChanParamREPR(p)+">";});

    py::class_<PvarsVoltageClamp, PvarsCell> pvarsVolt(m_Misc, "_PvarsVoltageClamp");
    pvarsVolt.def(py::init<Protocol*>(),py::keep_alive<1,2>())
        .def("__repr__",[pvarscell_REPR](PvarsVoltageClamp& c){
            return "PvarsVoltageClamp("+pvarscell_REPR(c)+")";
        });
    py::class_<PvarsVoltageClamp::SIonChanParam, PvarsCell::IonChanParam>(
        pvarsVolt, "_SIonChanParam")
        .def(py::init<>())
        .def_readwrite("val",&PvarsVoltageClamp::SIonChanParam::paramVal)
        .def("__repr__",[IonChanParamREPR](PvarsVoltageClamp::SIonChanParam& p){return "<SIonChanParam: "+IonChanParamREPR(p)+">";});

    py::class_<PvarsGrid, PvarsCell> pvarsGrid(m_Misc, "_PvarsGrid");
    pvarsGrid.def(py::init<Grid*>(),py::keep_alive<1,2>())
        .def("setMaxDistAndVal",&PvarsGrid::setMaxDistAndVal,"Set the maximum distance and the maximum value for a rule",py::arg("name"),py::arg("maxDist"),py::arg("maxVal"))
        .def("setStartCells",&PvarsGrid::setStartCells,"Set the cell locations that the rule will start from",py::arg("name"),py::arg("startCells"))
        .def("__repr__",[pvarscell_REPR](PvarsGrid& c){
            return "PvarsGrid("+pvarscell_REPR(c)+")";
        });
    py::class_<PvarsGrid::MIonChanParam, PvarsCell::IonChanParam>(
        pvarsGrid, "_MIonChanParam")
        .def(py::init<>())
        .def_readwrite("maxDist",&PvarsGrid::MIonChanParam::maxDist)
        .def_readwrite("maxVal",&PvarsGrid::MIonChanParam::maxVal)
        .def_readwrite("startCells",&PvarsGrid::MIonChanParam::startCells)
        .def_readwrite("cells",&PvarsGrid::MIonChanParam::cells)
        .def("__repr__",[IonChanParamREPR](PvarsGrid::MIonChanParam& p){return "<MIonChanParam: "+IonChanParamREPR(p)+">";});

    py::class_<Measure>(m_Misc, "_Measure","Class that measures an individual variable from a cell")
        .def(py::init<>())
        .def("measure",&Measure::measure)
        .def("reset",&Measure::reset)
        .def_property_readonly("variables",&Measure::variables)
        .def_property_readonly("variablesMap",&Measure::variablesMap)
        .def_property("selection",(set<string>(Measure::*)(void))&Measure::selection,(void(Measure::*)(set<string>))&Measure::selection)
        .def("nameString",&Measure::getNameString)
        .def("valueString",&Measure::getValueString);
    py::class_<MeasureWave,Measure>(m_Misc, "_MeasureWave")
        .def(py::init<>())
        .def_property("percrepol",(double(MeasureWave::*)(void)const)&MeasureWave::percrepol,(void(MeasureWave::*)(double))&MeasureWave::percrepol);

    py::class_<MeasureManager>(m_Misc, "_MeasureManager","Manages measuring variables from a cell during a simulation. Can record per cycle measurements of min,peak,etc")
        .def(py::init<shared_ptr<Cell>>())
        .def_property("selection",(map<string,set<string>>(MeasureManager::*)(void))&MeasureManager::selection,(void(MeasureManager::*)(map<string,set<string>>))&MeasureManager::selection,"variables to be measured and their measured properties")
        .def_property("percrepol",(double(MeasureManager::*)(void))&MeasureManager::percrepol,(void(MeasureManager::*)(double))&MeasureManager::percrepol,"the percent repolarization to write out measurements")
        .def("createMeasure",&MeasureManager::getMeasure,"Create a Measure",py::arg("varname"),py::arg("selection"))
        .def("addMeasure",&MeasureManager::addMeasure,"Add a new cell variable to be measured",py::arg("varname"),py::arg("selection")=set<string>())
        .def("setupMeasures",&MeasureManager::setupMeasures,"Get measures ready for simulation",py::arg("filename"))
        .def("measure",&MeasureManager::measure,"measure the variables",py::arg("time"))
        .def("resetMeasures",&MeasureManager::resetMeasures,"reset measures that are invalid after percrepol is reached")
        .def_readonly("varsMeas",&MeasureManager::varsMeas)
        .def_readonly("varMeasCreator",&MeasureManager::varMeasCreator);
    py::class_<GridMeasureManager,MeasureManager>(m_Misc, "_GridMeasureManager")
        .def(py::init<shared_ptr<GridCell>>())
        .def_property("dataNodes",(set<pair<int,int>>(GridMeasureManager::*)(void))&GridMeasureManager::dataNodes,(void(GridMeasureManager::*)(set<pair<int,int>>))&GridMeasureManager::dataNodes);

    py::class_<SettingsIO>(m_Misc, "SettingsIO")
        .def("getInstance",&SettingsIO::getInstance,py::return_value_policy::reference)
        .def("readSettings", [](SettingsIO& s, char* filename, shared_ptr<Protocol> proto = NULL){return s.readSettings(filename, proto);}, py::arg("filename"), py::arg("proto") = NULL)
        .def("writeSettings", [](SettingsIO& s,char* filename, shared_ptr<Protocol> proto){s.writeSettings(filename, proto);}, py::arg("filename"), py::arg("proto"));

    py::class_<RunSim>(m_Misc, "RunSim", "Runs simulations in a multithreaded environment")
	.def(py::init<shared_ptr<Protocol>>())
	.def(py::init<vector<shared_ptr<Protocol>>>())
	.def("run",&RunSim::run,"Run simulations in parallel")
	.def("finished",&RunSim::finished,"Check if simulations are all finished")
	.def("progress",&RunSim::progress,"print progress message")
	.def("setSims",(void(RunSim::*)(std::vector<shared_ptr<Protocol>>))&RunSim::setSims,"Set simualtions to run")
	.def("setSims",(void(RunSim::*)(shared_ptr<Protocol>))&RunSim::setSims,"Set simualtions to run")
	.def("appendSims",(void(RunSim::*)(std::vector<shared_ptr<Protocol>>))&RunSim::appendSims,"Append simulations to run")
	.def("appendSims",(void(RunSim::*)(shared_ptr<Protocol>))&RunSim::appendSims,"Append simulations to run")
	.def("clear",&RunSim::clear,"Clear list of simulations to run");
}
