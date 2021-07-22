#include "gridmeasuremanager.h"
#include "measure.h"
#include "measuredefault.h"
#include "measurefactory.h"
#include "measuremanager.h"
#include "measurevoltage.h"
#include "pylongqt.h"

#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

using namespace LongQt;
using std::string;
using std::set;
using std::shared_ptr;
using std::map;
using std::pair;

void init_measures(py::module& m) {
  py::module m_Meas = m.def_submodule("Measures", "Classes to measure cell variables throughout a simulation");

  py::class_<Measure>(m_Meas, "Measure",
                      "Base class for measuring values during a simulation, one Measure is needed per cell variable")
      .def("measure", &Measure::measure, "Measure values given the next time-point")
      .def("reset", &Measure::reset, "Reset the measure and ready it for more time-points")
      .def_property_readonly("variables", &Measure::variables, R"pbdoc(The names of the measured values being tracked (eg min, peak))pbdoc")
      .def_property_readonly("variablesMap", &Measure::variablesMap,
                             "pbdoc(The measured names and values being tracked (eg min, peak))pbdoc")
      .def_property("selection",
                    (set<string>(Measure::*)(void)) & Measure::selection,
                    (void (Measure::*)(set<string>)) & Measure::selection,
                    "The selection of measured values to be written out")
      .def("nameString", &Measure::getNameString, "The names the selected measured values")
      .def("valueString", &Measure::getValueString, "The current values of the of the selection");
  py::class_<MeasureDefault, Measure>(
      m_Meas, "MeasureDefault",
      R"pbdoc(Class that measures values for any cell variable that is not the transmembrane voltage (vOld))pbdoc")
      .def(py::init<>());
  py::class_<MeasureVoltage, Measure>(m_Meas, "MeasureVoltage",
      R"pbdoc(Class that measures values for the transmembrane voltage (vOld))pbdoc")
      .def(py::init<>())
      .def_property(
          "percrepol",
          (double (MeasureVoltage::*)(void) const) & MeasureVoltage::percrepol,
          (void (MeasureVoltage::*)(double)) & MeasureVoltage::percrepol,
              R"pbdoc(The percent repolarization at which an action potential ends)pbdoc");
  py::class_<MeasureFactory>(m_Meas, "MeasureFactory",
      "Creates the appropriate Measure given a variable name")
      .def(py::init<>())
      .def_property(
          "percrepol",
          (double (MeasureFactory::*)(void)) & MeasureFactory::percrepol,
          (void (MeasureFactory::*)(double)) & MeasureFactory::percrepol,
          R"pbdoc(The percent repolarization at which an action potential ends)pbdoc")
      .def("buildMeasure", &MeasureFactory::buildMeasure,
           R"pbdoc(Build the appropriate measure
           :varname: The cell variable name
           :selection: The selection of values to be measured)pbdoc",
           py::arg("varname"), py::arg("selection") = std::set<std::string>())
      .def("measureType", &MeasureFactory::measureType,
           R"pbdoc(Returns the name of the measure class for that variable name
           :varname: The cell variable's name)pbdoc",
           py::arg("varname"))
      .def("measureOptions", &MeasureFactory::measureOptions,
           R"pbdoc(Returns the values that can be measured by a measure class
           :measType: The name of the measure class)pbdoc",
           py::arg("measType"));

  py::class_<MeasureManager>(m_Meas, "MeasureManager",
                             R"pbdoc(Manages all the Measure objects for a simulation.
                             )pbdoc")
      .def(py::init<shared_ptr<Cell>>())
      .def_property("selection",
                    (map<string, set<string>>(MeasureManager::*)(void)) &
                        MeasureManager::selection,
                    (void (MeasureManager::*)(map<string, set<string>>)) &
                        MeasureManager::selection,
                    R"pbdoc(The selection of variables to measure and which values
                    should be measured for each of those variables)pbdoc")
      .def_property(
          "percrepol",
          (double (MeasureManager::*)(void)) & MeasureManager::percrepol,
          (void (MeasureManager::*)(double)) & MeasureManager::percrepol,
          R"pbdoc(The percent repolarization at which an action potential ends)pbdoc")
      .def("addMeasure", &MeasureManager::addMeasure,
           R"pbdoc(Add a new cell variable to be measured
           :varname: The cell variable's name
           :selection: The variable's values to be measured)pbdoc",
           py::arg("varname"), py::arg("selection") = set<string>())
      .def("setupMeasures", &MeasureManager::setupMeasures,
           "Get measures ready for simulation")
      .def("measure", &MeasureManager::measure,
           R"pbdoc(Measure all of the variables
           :time: The current simulation time (ms)
           :write: Should the measured values be saved)pbdoc",
           py::arg("time"), py::arg("write") = false);
  py::class_<GridMeasureManager, MeasureManager>(m_Meas, "GridMeasureManager",
        "The measure manager for grid simulations")
      .def(py::init<shared_ptr<GridCell>>())
      .def_property("dataNodes",
                    (set<pair<int, int>>(GridMeasureManager::*)(void)) &
                        GridMeasureManager::dataNodes,
                    (void (GridMeasureManager::*)(set<pair<int, int>>)) &
                        GridMeasureManager::dataNodes,
                    "The nodes on which measurements will be made");
}
