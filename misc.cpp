#include "cellpvars.h"
#include "measure.h"
#include "settingsIO.h"
#include "pvarscurrentclamp.h"
#include "pvarsvoltageclamp.h"
#include "pvarsgrid.h"
#include "pylongqt.h"
#include "measurewave.h"
#include "measuremanager.h"
#include "gridmeasuremanager.h"


void init_misc(py::module &m) {
    py::module m_Misc = m.def_submodule("Misc", "Measure, Pvars, helper classes");

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
        .def("__getitem__",&CellPvars::at,py::return_value_policy::reference_internal)
        .def("__iter__", [](const CellPvars &pvars)
            { return py::make_iterator(pvars.begin(), pvars.end(),py::return_value_policy::reference_internal); },
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
    pvarsCurr.def(py::init<Protocol*>(),py::keep_alive<1,2>())
        .def("protocol",&PvarsCurrentClamp::protocol,py::keep_alive<1,2>())
        .def("__repr__",[cellPvarsREPR](PvarsCurrentClamp& c){
            return "PvarsCurrentClamp("+cellPvarsREPR(c)+")";
        });
    py::class_<PvarsCurrentClamp::TIonChanParam, CellPvars::IonChanParam>(
        pvarsCurr, "TIonChanParam")
        .def(py::init<>())
        .def_readwrite("trials",&PvarsCurrentClamp::TIonChanParam::trials)
        .def("__repr__",[IonChanParamREPR](PvarsCurrentClamp::TIonChanParam& p){return "<TIonChanParam: "+IonChanParamREPR(p)+">";});

    py::class_<PvarsVoltageClamp, CellPvars> pvarsVolt(m_Misc, "PvarsVoltageClamp");
    pvarsVolt.def(py::init<Protocol*>(),py::keep_alive<1,2>())
        .def("protocol",&PvarsVoltageClamp::protocol,py::keep_alive<1,2>())
        .def("__repr__",[cellPvarsREPR](PvarsVoltageClamp& c){
            return "PvarsVoltageClamp("+cellPvarsREPR(c)+")";
        });
    py::class_<PvarsVoltageClamp::SIonChanParam, CellPvars::IonChanParam>(
        pvarsVolt, "SIonChanParam")
        .def(py::init<>())
        .def_readwrite("val",&PvarsVoltageClamp::SIonChanParam::paramVal)
        .def("__repr__",[IonChanParamREPR](PvarsVoltageClamp::SIonChanParam& p){return "<SIonChanParam: "+IonChanParamREPR(p)+">";});

    py::class_<PvarsGrid, CellPvars> pvarsGrid(m_Misc, "PvarsGrid");
    pvarsGrid.def(py::init<Grid*>(),py::keep_alive<1,2>())
        .def("setGrid",&PvarsGrid::setGrid,py::keep_alive<1,2>())
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
        .def("setGridCell",(void(GridMeasureManager::*)(shared_ptr<GridCell>))&GridMeasureManager::cell);

    py::class_<SettingsIO>(m_Misc, "SettingsIO")
        .def("getInstance",&SettingsIO::getInstance)
    //            .def("writedvars",&SettingsIO::writedvars)
    //            .def("readdvars",&SettingsIO::readdvars)
        .def_readonly("lastProto",&SettingsIO::lastProto)
        .def_readwrite("allowProtoChange",&SettingsIO::allowProtoChange)
        .def("readSettings", [](SettingsIO& s,shared_ptr<Protocol> proto,char* filename){s.readSettings(proto,filename);})
        .def("writeSettings", [](SettingsIO& s,shared_ptr<Protocol> proto,char* filename){s.writeSettings(proto,filename);});
}