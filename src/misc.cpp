#include "datareader.h"
#include "gridmeasuremanager.h"
#include "measure.h"
#include "measuredefault.h"
#include "measurefactory.h"
#include "measuremanager.h"
#include "measurevoltage.h"
#include "pvarscell.h"
#include "pvarscurrentclamp.h"
#include "pvarsgrid.h"
#include "pvarsvoltageclamp.h"
#include "pylongqt.h"
#include "runsim.h"
#include "settingsIO.h"

#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

PYBIND11_MAKE_OPAQUE(std::vector<LongQt::DataReader::TrialData<LongQt::DataReader::TraceHeader>>);
PYBIND11_MAKE_OPAQUE(std::vector<LongQt::DataReader::TrialData<LongQt::DataReader::MeasHeader>>);

PYBIND11_MAKE_OPAQUE(std::vector<LongQt::DataReader::TraceHeader>);
PYBIND11_MAKE_OPAQUE(std::vector<LongQt::DataReader::MeasHeader>);

using namespace LongQt;
using namespace std;

void init_misc(py::module& m) {
  py::module m_Misc = m.def_submodule("Misc", "Measure, Pvars, helper classes");

  auto pvarscell_REPR = [](PvarsCell& p) {
    string out = "{";
    bool first = true;
    for (auto& par : p) {
      if (!first) out += ", ";
      out += "'" + par.first + "': " + string(py::repr(py::cast(par.second)));
      first = false;
    }
    out += "}";
    return out;
  };
  py::class_<PvarsCell> pvarscell(m_Misc, "_PvarsCell",
                                  "Sets cell constant values");
  pvarscell
      .def("setIonChanParams", &PvarsCell::setIonChanParams,
           "apply ion channel parameters")
      .def("calcIonChanParams", &PvarsCell::calcIonChanParams,
           "calculate the ion channel parameters")
      .def("__setitem__", &PvarsCell::insert, "insert new rule",
           py::arg("name"), py::arg("parameter"))
      .def("__delitem__", &PvarsCell::erase)
      .def("clear", &PvarsCell::clear, "remove all rules")
      .def("__getitem__", &PvarsCell::at,
           py::return_value_policy::reference_internal)
      .def("__iter__",
           [](const PvarsCell& pvars) {
             return py::make_iterator(
                 pvars.begin(), pvars.end(),
                 py::return_value_policy::reference_internal);
           },
           py::keep_alive<0, 1>())
      .def("__len__", &PvarsCell::size)
      .def("__repr__", pvarscell_REPR);
  py::enum_<PvarsCell::Distribution>(pvarscell, "Distribution")
      .value("none", PvarsCell::Distribution::none)
      .value("normal", PvarsCell::Distribution::normal)
      .value("lognormal", PvarsCell::Distribution::lognormal)
      .export_values();
  auto IonChanParamREPR = [](PvarsCell::IonChanParam& p) {
    auto s = QString(CellUtils::trim(p.IonChanParam::str("")).c_str());
    auto split = s.split(QRegExp("\\t"), QString::SkipEmptyParts);
    return (split[0] + " (" + split[1] + " " + split[2] + ")").toStdString();
  };
  py::class_<PvarsCell::IonChanParam>(pvarscell, "IonChanParam",
                                      "A rule for a cell constant")
      .def(py::init<PvarsCell::Distribution, double, double>(),
           py::arg("dist") = PvarsCell::Distribution::none, py::arg("val1") = 0,
           py::arg("val2") = 0)
      .def_readwrite("dist", &PvarsCell::IonChanParam::dist)
      .def_property(
          "val0",
          [](const PvarsCell::IonChanParam& param) { return param.val[0]; },
          [](PvarsCell::IonChanParam& param, double val) {
            param.val[0] = val;
          },
          "Depends on distribution\n none: starting value\n normal & "
          "lognormal: mean")
      .def_property(
          "val1",
          [](const PvarsCell::IonChanParam& param) { return param.val[1]; },
          [](PvarsCell::IonChanParam& param, double val) {
            param.val[1] = val;
          },
          "Depends on distribution\nnone: increment amount\nnormal & "
          "lognormal: standard deviation")
      .def("__repr__", IonChanParamREPR);

  py::class_<PvarsCurrentClamp, PvarsCell> pvarsCurr(m_Misc,
                                                     "_PvarsCurrentClamp");
  pvarsCurr.def(py::init<Protocol*>(), py::keep_alive<1, 2>())
      .def("__repr__", [pvarscell_REPR](PvarsCurrentClamp& c) {
        return "PvarsCurrentClamp(" + pvarscell_REPR(c) + ")";
      });
  py::class_<PvarsCurrentClamp::TIonChanParam, PvarsCell::IonChanParam>(
      pvarsCurr, "_TIonChanParam")
      .def(py::init<>())
      .def_readwrite("trials", &PvarsCurrentClamp::TIonChanParam::trials)
      .def("__repr__", [IonChanParamREPR](PvarsCurrentClamp::TIonChanParam& p) {
        return "<TIonChanParam: " + IonChanParamREPR(p) + ">";
      });

  py::class_<PvarsVoltageClamp, PvarsCell> pvarsVolt(m_Misc,
                                                     "_PvarsVoltageClamp");
  pvarsVolt.def(py::init<Protocol*>(), py::keep_alive<1, 2>())
      .def("__repr__", [pvarscell_REPR](PvarsVoltageClamp& c) {
        return "PvarsVoltageClamp(" + pvarscell_REPR(c) + ")";
      });
  py::class_<PvarsVoltageClamp::SIonChanParam, PvarsCell::IonChanParam>(
      pvarsVolt, "_SIonChanParam")
      .def(py::init<>())
      .def_readwrite("val", &PvarsVoltageClamp::SIonChanParam::paramVal)
      .def("__repr__", [IonChanParamREPR](PvarsVoltageClamp::SIonChanParam& p) {
        return "<SIonChanParam: " + IonChanParamREPR(p) + ">";
      });

  py::class_<PvarsGrid, PvarsCell> pvarsGrid(m_Misc, "_PvarsGrid");
  pvarsGrid.def(py::init<Grid*>(), py::keep_alive<1, 2>())
      .def("setMaxDistAndVal", &PvarsGrid::setMaxDistAndVal,
           "Set the maximum distance and the maximum value for a rule",
           py::arg("name"), py::arg("maxDist"), py::arg("maxVal"))
      .def("setStartCells", &PvarsGrid::setStartCells,
           "Set the cell locations that the rule will start from",
           py::arg("name"), py::arg("startCells"))
      .def("__repr__", [pvarscell_REPR](PvarsGrid& c) {
        return "PvarsGrid(" + pvarscell_REPR(c) + ")";
      });
  py::class_<PvarsGrid::MIonChanParam, PvarsCell::IonChanParam>(
      pvarsGrid, "_MIonChanParam")
      .def(py::init<>())
      .def_readwrite("maxDist", &PvarsGrid::MIonChanParam::maxDist)
      .def_readwrite("maxVal", &PvarsGrid::MIonChanParam::maxVal)
      .def_readwrite("startCells", &PvarsGrid::MIonChanParam::startCells)
      .def_readwrite("cells", &PvarsGrid::MIonChanParam::cells)
      .def("__repr__", [IonChanParamREPR](PvarsGrid::MIonChanParam& p) {
        return "<MIonChanParam: " + IonChanParamREPR(p) + ">";
      });

  py::class_<Measure>(m_Misc, "_Measure",
                      "Base class for measure values during a simulation")
      .def("measure", &Measure::measure)
      .def("reset", &Measure::reset)
      .def_property_readonly("variables", &Measure::variables)
      .def_property_readonly("variablesMap", &Measure::variablesMap)
      .def_property("selection",
                    (set<string>(Measure::*)(void)) & Measure::selection,
                    (void (Measure::*)(set<string>)) & Measure::selection)
      .def("nameString", &Measure::getNameString)
      .def("valueString", &Measure::getValueString);
  py::class_<MeasureDefault, Measure>(
      m_Misc, "_MeasureDefault",
      "Class that measures an individual variable from a cell")
      .def(py::init<>());
  py::class_<MeasureVoltage, Measure>(m_Misc, "_MeasureVoltage")
      .def(py::init<>())
      .def_property(
          "percrepol",
          (double (MeasureVoltage::*)(void) const) & MeasureVoltage::percrepol,
          (void (MeasureVoltage::*)(double)) & MeasureVoltage::percrepol);
  py::class_<MeasureFactory>(
      m_Misc, "_MeasureFactory",
      "Creates appropriate Measure given a variable name")
      .def(py::init<>())
      .def_property(
          "percrepol",
          (double (MeasureFactory::*)(void)) & MeasureFactory::percrepol,
          (void (MeasureFactory::*)(double)) & MeasureFactory::percrepol,
          "the percent repolarization to write out measurements")
      .def("buildMeasure", &MeasureFactory::buildMeasure, "Build a Measure",
           py::arg("varname"), py::arg("selection") = std::set<std::string>())
      .def("measureType", &MeasureFactory::measureType,
           "Returns the name of the measure class for the variable",
           py::arg("varname"))
      .def("measureOptions", &MeasureFactory::measureOptions,
           "Returns the measures available for a Measure type",
           py::arg("measType"));

  py::class_<MeasureManager>(m_Misc, "_MeasureManager",
                             "Manages measuring variables from a cell "
                             "during a simulation. Can record "
                             "per cycle measurements of min,peak,etc")
      .def(py::init<shared_ptr<Cell>>())
      .def_property("selection",
                    (map<string, set<string>>(MeasureManager::*)(void)) &
                        MeasureManager::selection,
                    (void (MeasureManager::*)(map<string, set<string>>)) &
                        MeasureManager::selection,
                    "variables to be measured and their measured properties")
      .def_property(
          "percrepol",
          (double (MeasureManager::*)(void)) & MeasureManager::percrepol,
          (void (MeasureManager::*)(double)) & MeasureManager::percrepol,
          "the percent repolarization to write out measurements")
      .def("addMeasure", &MeasureManager::addMeasure,
           "Add a new cell variable to be measured", py::arg("varname"),
           py::arg("selection") = set<string>())
      .def("setupMeasures", &MeasureManager::setupMeasures,
           "Get measures ready for simulation")
      .def("measure", &MeasureManager::measure, "measure the variables",
           py::arg("time"), py::arg("write") = false);
  py::class_<GridMeasureManager, MeasureManager>(m_Misc, "_GridMeasureManager")
      .def(py::init<shared_ptr<GridCell>>())
      .def_property("dataNodes",
                    (set<pair<int, int>>(GridMeasureManager::*)(void)) &
                        GridMeasureManager::dataNodes,
                    (void (GridMeasureManager::*)(set<pair<int, int>>)) &
                        GridMeasureManager::dataNodes);

  py::class_<SettingsIO>(m_Misc, "SettingsIO")
      .def_static("getInstance", &SettingsIO::getInstance,
                  py::return_value_policy::reference)
      .def(
          "readSettings",
          [](SettingsIO& s, char* filename, shared_ptr<Protocol> proto = nullptr) {
            //            py::
            //            py::scoped_ostream_redirect stream(
            //                std::cout,                                //
            //                std::ostream&
            //                py::module::import("sys").attr("stdout")  //
            //                Python output
            //            );
            return s.readSettings(filename, proto);
          },
          py::arg("filename"), py::arg("proto") = shared_ptr<Protocol>(nullptr))
      .def("writeSettings",
           [](SettingsIO& s, char* filename, shared_ptr<Protocol> proto) {
             s.writeSettings(filename, proto);
           },
           py::arg("filename"), py::arg("proto"));

  py::class_<RunSim>(m_Misc, "RunSim",
                     "Runs simulations in a multithreaded environment")
      .def(py::init())
      .def(py::init<shared_ptr<Protocol>>())
      .def(py::init<vector<shared_ptr<Protocol>>>())
      .def("run", &RunSim::run, "Run simulations in parallel")
      .def("cancel", &RunSim::cancel, "Cancel running simulations")
      .def("finished", &RunSim::finished,
           "Check if simulations are all finished")
      .def("progressRange", &RunSim::progressRange,
           "return the min and max progress values")
      .def("progress", &RunSim::progress, "Return progress")
      .def("progressPercent",
           [](RunSim& r) {
             auto mM = r.progressRange();
             return 100 * ((r.progress() - mM.first) / mM.second);
           })
      .def("startCallback", &RunSim::startCallback,
           "Callback function when run is called")
      .def("finishedCallback", &RunSim::finishedCallback,
           "Callback function when simulations are finished")
      .def("setSims",
           (void (RunSim::*)(std::vector<shared_ptr<Protocol>>)) &
               RunSim::setSims,
           "Set simualtions to run")
      .def("setSims",
           (void (RunSim::*)(shared_ptr<Protocol>)) & RunSim::setSims,
           "Set simualtions to run")
      .def("appendSims",
           (void (RunSim::*)(std::vector<shared_ptr<Protocol>>)) &
               RunSim::appendSims,
           "Append simulations to run")
      .def("appendSims",
           (void (RunSim::*)(shared_ptr<Protocol>)) & RunSim::appendSims,
           "Append simulations to run")
      .def("clear", &RunSim::clear, "Clear list of simulations to run");

  py::bind_vector<vector<DataReader::TrialData<DataReader::TraceHeader>>>(m, "VectorTrace")
          .def("__repr__", [m](const vector<DataReader::TrialData<DataReader::TraceHeader>>& self){
      unsigned int max_lines = m.attr("max_print_lines").cast<unsigned int>();
      std::string out = "[";
      unsigned int i = 0;
      for(; i < max_lines && i < self.size(); ++i) {
          py::object py_header = py::cast(self[i]);
          out += py::repr(py_header).cast<std::string>() + ",\n";
      }
      if(i == max_lines && max_lines < self.size()) {
          out += "...";
      }
      out += "]";
      return out;
  });
  py::bind_vector<vector<DataReader::TrialData<DataReader::MeasHeader>>>(m, "VectorMeas")
          .def("__repr__", [m](const vector<DataReader::TrialData<DataReader::MeasHeader>>& self){
      unsigned int max_lines = m.attr("max_print_lines").cast<unsigned int>();
      std::string out = "[";
      unsigned int i = 0;
      for(; i < max_lines && i < self.size(); ++i) {
          py::object py_header = py::cast(self[i]);
          out += py::repr(py_header).cast<std::string>() + ",\n";
      }
      if(i == max_lines && max_lines < self.size()) {
          out += "...";
      }
      out += "]";
      return out;
  });

  py::bind_vector<vector<DataReader::TraceHeader>>(m, "VectorTraceHeader")
          .def("__repr__", [m](const vector<DataReader::TraceHeader>& self){
      unsigned int max_lines = m.attr("max_print_lines").cast<unsigned int>();
      std::string out = "[";
      unsigned int i = 0;
      for(; i < max_lines && i < self.size(); ++i) {
          py::object py_header = py::cast(self[i]);
          out += py::repr(py_header).cast<std::string>() + ",\n";
      }
      if(i == max_lines && max_lines < self.size()) {
          out += "...";
      }
      out += "]";
      return out;
  });
  py::bind_vector<vector<DataReader::MeasHeader>>(m, "VectorMeasHeader")
          .def("__repr__", [m](const vector<DataReader::MeasHeader>& self){
      unsigned int max_lines = m.attr("max_print_lines").cast<unsigned int>();
      std::string out = "[";
      unsigned int i = 0;
      for(; i < max_lines && i < self.size(); ++i) {
          py::object py_header = py::cast(self[i]);
          out += py::repr(py_header).cast<std::string>() + ",\n";
      }
      if(i == max_lines && max_lines < self.size()) {
          out += "...";
      }
      out += "]";
      return out;
  });

  py::class_<DataReader> data_reader(m_Misc, "DataReader",
                                     "Reads data written by a simulation");
  data_reader
      .def_static("readFile",
                  [](const std::string& s, const std::set<int>& exclude = {}) {
                    return DataReader::readFile(s, exclude);
                  },
                  "Reads a single data file", py::arg("file"),
                  py::arg("exclude") = std::set<int>())
      .def_static("readDir",
                  [](const std::string& s, const std::set<int>& exclude = {}) {
                    return DataReader::readDir(s, exclude);
                  },
                  "Reads an entire simulation", py::arg("dir"),
                  py::arg("excludeTrials") = std::set<int>())
      .def_static(
          "getTrialNums",
          [](const std::string& s) { return DataReader::getTrialNums(s); },
          "Get the trial numbers from a directory", py::arg("dir"));
  py::class_<DataReader::TSVData>(data_reader, "TSVData",
                                  "Holds data read by one file")
      .def("__repr__", [](const DataReader::TSVData& self) {
            return "<TraceHeader for trial="+to_string(self.trial)+"with header, data["
                    +to_string(self.data.size())+"]>";
        })
      .def_readonly("header", &DataReader::TSVData::header)
      .def_readwrite("data", &DataReader::TSVData::data)
      .def_readonly("trial", &DataReader::TSVData::trial);
  py::class_<DataReader::MeasHeader>(data_reader, "MeasHeader",
                                     "The header entry for measured data")
      .def("__repr__", [](const DataReader::MeasHeader& self) {
            return "<TraceHeader for var="+self.var_name+", prop="+self.prop_name+">";
        })
      .def_readonly("cell_info", &DataReader::MeasHeader::cell_info)
      .def_readonly("cell_info_parsed",
                    &DataReader::MeasHeader::cell_info_parsed)
      .def_readonly("var_name", &DataReader::MeasHeader::var_name)
      .def_readonly("prop_name", &DataReader::MeasHeader::prop_name);
  py::class_<DataReader::TraceHeader>(data_reader, "TraceHeader",
                                      "The header entry for trace data")
      .def("__repr__", [](const DataReader::TraceHeader& self) {
            return "<TraceHeader for var="+self.var_name+">";
        })
      .def_readonly("cell_info", &DataReader::TraceHeader::cell_info)
      .def_readonly("cell_info_parsed",
                    &DataReader::TraceHeader::cell_info_parsed)
      .def_readonly("var_name", &DataReader::TraceHeader::var_name);
  py::class_<DataReader::TrialData<DataReader::MeasHeader>>(
      data_reader, "MeasData", "Header and data from a measure file")
      .def("__repr__", [](const DataReader::TrialData<DataReader::MeasHeader>& self) {
            return "<MeasData with header, data["+to_string(self.data.size())+"]>";
        })
      .def_readonly("header",
                    &DataReader::TrialData<DataReader::MeasHeader>::header,
                    py::return_value_policy::reference_internal)
      .def_readonly("data",
                    &DataReader::TrialData<DataReader::MeasHeader>::data,
                    py::return_value_policy::reference_internal);
  py::class_<DataReader::TrialData<DataReader::TraceHeader>>(
      data_reader, "TraceData", "Header and data from a trace file")
      .def("__repr__", [](const DataReader::TrialData<DataReader::TraceHeader>& self) {
            return "<TraceData with header, data["+to_string(self.data.size())+"]>";
        })
      .def_readonly("header",
                    &DataReader::TrialData<DataReader::TraceHeader>::header,
                    py::return_value_policy::reference_internal)
      .def_readonly("data",
                    &DataReader::TrialData<DataReader::TraceHeader>::data,
                    py::return_value_policy::reference_internal);
  py::class_<DataReader::SimData>(data_reader, "SimData",
                                  "Data from an entire simulation")
      .def("__repr__", [](const DataReader::SimData& self) {
            return "<SimData with meas["+to_string(self.meas.size())
                +"], trace["+to_string(self.trace.size())+"]>";
        })
      .def_readwrite("trace", &DataReader::SimData::trace,
                     py::return_value_policy::reference_internal)
      .def_readwrite("meas", &DataReader::SimData::meas,
                     py::return_value_policy::reference_internal);
}
