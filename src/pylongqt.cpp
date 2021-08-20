#include "pylongqt.h"
#include <iostream>
#include <string>
#include "cellutils.h"
#include "inexcitablecell.h"
#include "logger.h"
#include "measuredefault.h"
#include "measurefactory.h"
#include "measuremanager.h"
#include "protocol.h"
#include "runsim.h"
#include "datareader.h"

#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/stl_bind.h>

#include "settingsIO.h"

using namespace LongQt;
using std::make_shared;
using std::map;
using std::set;
using std::string;
using std::shared_ptr;
using std::vector;
using std::to_string;

PYBIND11_MAKE_OPAQUE(std::vector<LongQt::DataReader::TrialData<LongQt::DataReader::TraceHeader>>);
PYBIND11_MAKE_OPAQUE(std::vector<LongQt::DataReader::TrialData<LongQt::DataReader::MeasHeader>>);

PYBIND11_MAKE_OPAQUE(std::vector<LongQt::DataReader::TraceHeader>);
PYBIND11_MAKE_OPAQUE(std::vector<LongQt::DataReader::MeasHeader>);

/*auto measMap = [](string name) {
  auto meas = new MeasureFactory;
  map<string, set<string>> measMap;
  for (auto var : meas->varsMeas) {
    measMap[var.first] = meas->varMeasCreator.at(var.second)({})->variables();
  }
  set<string> defaultMeas = MeasureDefault().variables();
  delete meas;
  return measMap.count(name) > 0 ? measMap[name] : defaultMeas;
};*/

void setVerbose(bool on) {
    on ? Logger::getInstance()
             ->STDOut(new std::ostream(new
                 py::detail::pythonbuf(py::module::import("sys").attr("stdout"))))
        : Logger::getInstance()->delSTDOut();
}

PYBIND11_MODULE(_PyLongQt, m) {
  auto logger = Logger::getInstance();
  logger->exceptionEnabled = true;

  m.doc() = R"pbdoc(
          Python bindings for LongQt, a cardiac cell electrophysiology simulation platform.
          This library allows for cell models to be created and configured to run both
          simple and complex simulations.

          .. Note::
            Further documentation on LongQt and its graphical interface as well as sample code can be found in
            the `LongQt manual <https://www.longqt.readthedocs.io>`_
        )pbdoc";

  m.attr("cellMap") =
      py::cast(CellUtils::cellMap, py::return_value_policy::take_ownership);
  m.attr("protoMap") =
      py::cast(CellUtils::protoMap, py::return_value_policy::take_ownership);
//  m.def("measMap", measMap,
//        "Get the available measure variables for a given cell variable",
//        py::arg("name"));
  py::enum_<CellUtils::Side>(m, "Side", "The sides of a cell in a 2D grids")
      .value("top", CellUtils::top)
      .value("bottom", CellUtils::bottom)
      .value("right", CellUtils::right)
      .value("left", CellUtils::left)
      .export_values();
  //this relies on a pybind11 detail
  m.def("verbose", &setVerbose, "Set whether all verbose warnings will be printed");
  m.attr("version") = CellUtils::version;
  m.attr("max_print_lines") = 20;

  init_cells(m);
  init_protocols(m);
  init_structures(m);
  init_measures(m);
  init_pvars(m);

  py::class_<SettingsIO>(m, "SettingsIO",
                         "The interface for reading and writing settings files.")
      .def_static("getInstance", &SettingsIO::getInstance,
                  R"pbdoc(Get the instance of the SettingsIO object)pbdoc",
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
           R"pbdoc(Read settings from a file
            :filename: The settings file
            :proto: The protocol the settings should be read into. If not set
             or if the settings and the protocol are not of the same type
             a new protocol will be created. )pbdoc",
          py::arg("filename"), py::arg("proto") = shared_ptr<Protocol>(nullptr))
      .def("writeSettings",
           [](SettingsIO& s, char* filename, shared_ptr<Protocol> proto) {
             s.writeSettings(filename, proto);
           },
          R"pbdoc(Write the settings from a protocol
          :filename: The file to store the settings in
          :proto: The proto with settings to be saved)pbdoc",
           py::arg("filename"), py::arg("proto"));

  py::class_<RunSim>(m, "RunSim",
                     "Runs simulations in a multithreaded environment")
      .def(py::init())
      .def(py::init<shared_ptr<Protocol>>())
      .def(py::init<vector<shared_ptr<Protocol>>>())
      .def("run", &RunSim::run, "Run simulations in parallel")
      .def("cancel", &RunSim::cancel, "Cancel running simulations")
      .def("finished", &RunSim::finished,
           "Check if simulations are all finished")
      .def("progressRange", &RunSim::progressRange,
           "Return the min and max possible progress values")
      .def("progress", &RunSim::progress, "Return simulation progress")
      .def("progressPercent",
           [](RunSim& r) {
             auto mM = r.progressRange();
             return 100 * ((r.progress() - mM.first) / mM.second);
           }, "Return simulation progress as a percentage")
      .def("startCallback", &RunSim::startCallback,
           R"pbdoc(Set a callback function for when RunSim begins running simulations
           :callback: The function that will be called before any simulations run)pbdoc",
           py::arg("callback"))
      .def("finishedCallback", &RunSim::finishedCallback,
           R"pbdoc(Set a callback function for when all simulations have finished
           :callback: The function that will be called after all simulations have been run)pbdoc",
           py::arg("callback"))
      .def("setSims",
           (void (RunSim::*)(std::vector<shared_ptr<Protocol>>)) &
               RunSim::setSims,
           R"pbdoc(Set simualtions to run, replacing any currently queued simulations
           :protos: A list of protocols to be run)pbdoc",
           py::arg("protos"))
      .def("setSims",
           (void (RunSim::*)(shared_ptr<Protocol>)) & RunSim::setSims,
           R"pbdoc(Set simualtions to run, replacing any currently queued simulations
           :proto: A protocol to be run)pbdoc",
           py::arg("proto"))
      .def("appendSims",
           (void (RunSim::*)(std::vector<shared_ptr<Protocol>>)) &
               RunSim::appendSims,
           R"pbdoc(Append simulations to run to the queue of simulations
           :protos: A list of protocols to be run)pbdoc",
           py::arg("protos"))
      .def("appendSims",
           (void (RunSim::*)(shared_ptr<Protocol>)) & RunSim::appendSims,
           R"pbdoc(Append simulations to run to the queue of simulations
           :proto: A protocol to be run)pbdoc",
           py::arg("proto"))
      .def("clear", &RunSim::clear, "Clear queue of simulations");


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

  py::class_<DataReader> data_reader(m, "DataReader",
                                     "Reads saved data from simulations");
  data_reader
      .def_static("readFile",
                  [](const std::string& s, const std::set<int>& exclude = {}) {
                    return DataReader::readFile(s, exclude);
                  },
                  R"pbdoc(Read a single data file
                    :file: The file to read
                    :exclude_trials: Trial numbers not to be read)pbdoc",
                    py::arg("file"), py::arg("exclude_trials") = std::set<int>())
      .def_static("readDir",
                  [](const std::string& s, const std::set<int>& exclude = {}) {
                    return DataReader::readDir(s, exclude);
                  },
                  R"pbdoc(Read files for an entire simulation
                    :dir: The directory which contains the simulation
                    :exclude_trials: Trial numbers not to be read)pbdoc",
                    py::arg("dir"), py::arg("exclude_trials") = std::set<int>())
      .def_static(
          "getTrialNums",
          [](const std::string& s) { return DataReader::getTrialNums(s); },
          R"pbdoc(Read the trial numbers from a simulation
                    :dir: The directory which contains the simulation)pbdoc",
                    py::arg("dir"));
  py::class_<DataReader::TSVData>(data_reader, "TSVData",
                                  "Holds data read by one file")
      .def("__repr__", [](const DataReader::TSVData& self) {
            return "<TraceHeader for trial="+to_string(self.trial)+"with header, data["
                    +to_string(self.data.size())+"]>";
        })
      .def_readonly("header", &DataReader::TSVData::header, "The variable names for each data column")
      .def_readwrite("data", &DataReader::TSVData::data, "The data for each column")
      .def_readonly("trial", &DataReader::TSVData::trial, "The trial number for the file");
  py::class_<DataReader::MeasHeader>(data_reader, "MeasHeader",
                                     "The header entry for measured data")
      .def("__repr__", [](const DataReader::MeasHeader& self) {
            return "<MeasHeader for var="+self.var_name+", prop="+self.prop_name+">";
        })
      .def_readonly("cell_info", &DataReader::MeasHeader::cell_info,
                    R"pbdoc(Raw cell info structured as cell *row* _ *column* / *variable* / *measured property*)pbdoc")
      .def_readonly("cell_info_parsed",
                    &DataReader::MeasHeader::cell_info_parsed,
                    "Cell info parsed into a list, empty if simulation is not a grid")
      .def_readonly("var_name", &DataReader::MeasHeader::var_name, "The name of the variable")
      .def_readonly("prop_name", &DataReader::MeasHeader::prop_name, "The name of the measured property");
  py::class_<DataReader::TraceHeader>(data_reader, "TraceHeader",
                                      "The header entry for trace data")
      .def("__repr__", [](const DataReader::TraceHeader& self) {
            return "<TraceHeader for var="+self.var_name+">";
        })
      .def_readonly("cell_info", &DataReader::TraceHeader::cell_info,
                    R"pbdoc(Raw cell info structured as cell *row* _ *column* / *variable*)pbdoc")
      .def_readonly("cell_info_parsed",
                    &DataReader::TraceHeader::cell_info_parsed,
                    "Cell info parsed into a list, empty if simulation is not a grid")
      .def_readonly("var_name", &DataReader::TraceHeader::var_name, "The name of the variable");
  py::class_<DataReader::TrialData<DataReader::MeasHeader>>(
      data_reader, "MeasData", "Header and data for measured data")
      .def("__repr__", [](const DataReader::TrialData<DataReader::MeasHeader>& self) {
            return "<MeasData with header, data["+to_string(self.data.size())+"]>";
        })
      .def_readonly("header",
                    &DataReader::TrialData<DataReader::MeasHeader>::header,
                    "The headers for measured data, one entry for each measured property for each variable",
                    py::return_value_policy::reference_internal)
      .def_readonly("data",
                    &DataReader::TrialData<DataReader::MeasHeader>::data,
                    "The measured data, one entry for each measured property for each variable",
                    py::return_value_policy::reference_internal);
  py::class_<DataReader::TrialData<DataReader::TraceHeader>>(
      data_reader, "TraceData", "Header and data for trace data")
      .def("__repr__", [](const DataReader::TrialData<DataReader::TraceHeader>& self) {
            return "<TraceData with header, data["+to_string(self.data.size())+"]>";
        })
      .def_readonly("header",
                    &DataReader::TrialData<DataReader::TraceHeader>::header,
                    "The headers for trace data, one entry for each variable to be saved",
                    py::return_value_policy::reference_internal)
      .def_readonly("data",
                    &DataReader::TrialData<DataReader::TraceHeader>::data,
                    "The trace data, one entry for each variable",
                    py::return_value_policy::reference_internal);
  py::class_<DataReader::SimData>(data_reader, "SimData",
                                  "All the data from a simulation")
      .def("__repr__", [](const DataReader::SimData& self) {
            return "<SimData with meas["+to_string(self.meas.size())
                +"], trace["+to_string(self.trace.size())+"]>";
        })
      .def_readwrite("trace", &DataReader::SimData::trace,
                     "The saved traces",
                     py::return_value_policy::reference_internal)
      .def_readwrite("meas", &DataReader::SimData::meas,
                     "The saved measures",
                     py::return_value_policy::reference_internal);

  // py::register_exception<std::runtime_error>(m, "RuntimeException");

  setVerbose(true);
}
