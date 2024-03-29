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


#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>



using namespace LongQt;
using std::string;
using std::set;
using std::shared_ptr;
using std::map;
using std::pair;

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
  py::class_<PvarsCell> pvarscell(m_Misc, "PvarsCell",
                                  R"pbdoc(Base class for protocol specific pvars,
                                  pvars control cell parameters, which are values set at the
                                  beginning of a simulation and are not changed throughout. Different
                                  trials may have different parameter values. pvars provides a simple
                                  class to control these cell parameter settings.)pbdoc");
  pvarscell
      .def("setIonChanParams", &PvarsCell::setIonChanParams,
           "Apply ion channel parameters to cell(s)")
      .def("calcIonChanParams", &PvarsCell::calcIonChanParams,
           "Re-calculate the ion channel parameters")
      .def("__setitem__", &PvarsCell::insert,
           R"pbdoc(Insert a new rule to define cell parameter values
           :name: The cell parameter's name
           :parameter: The rule which defines how the cell parameter will be set)pbdoc",
           py::arg("name"), py::arg("parameter"))
      .def("__delitem__", &PvarsCell::erase,
           R"pbdoc(Delete a specific rule
           :name: The cell parameter's name)pbdoc",
           py::arg("name"))
      .def("clear", &PvarsCell::clear, "Delete all of the rules")
      .def("__getitem__", &PvarsCell::at,
           R"pbdoc(Retrieve the rule for a cell parameter
           :name: The cell parameter's name)pbdoc",
           py::arg("name"),
           py::return_value_policy::reference_internal)
      .def("__iter__",
           [](const PvarsCell& pvars) {
             return py::make_iterator(
                 pvars.begin(), pvars.end(),
                 py::return_value_policy::reference_internal);
           },
            "Iterate over cell parameters",
           py::keep_alive<0, 1>())
      .def("__len__", &PvarsCell::size, "Number of rules")
      .def("__repr__", pvarscell_REPR);
  py::enum_<PvarsCell::Distribution>(pvarscell, "Distribution",
                                     "Possible distributions for cell parameter rules")
      .value("none", PvarsCell::Distribution::none,
             "No distribution")
      .value("normal", PvarsCell::Distribution::normal,
             R"pbdoc(Gaussian distribution :math:`N(\mu, \sigma^2)`)pbdoc")
      .value("lognormal", PvarsCell::Distribution::lognormal,
             R"pbdoc(Log-Gaussian distribution :math:`exp(N(\mu, \sigma^2))`)pbdoc")
      .export_values();
  auto IonChanParamREPR = [](PvarsCell::IonChanParam& p) {
    auto s = QString(CellUtils::trim(p.IonChanParam::str("")).c_str());
    auto split = s.split(QRegExp("\\t"), QString::SkipEmptyParts);
    return (split[0] + " (" + split[1] + " " + split[2] + ")").toStdString();
  };
  py::class_<PvarsCell::IonChanParam>(pvarscell, "IonChanParam",
                                      R"pbdoc(
A rule for a cell parameter

Depending on the distribution the rule's values have different meanings

Distribution values:

    :none:
        :val1: The starting value
        :val2: The increment amount for each trial (may be positive or negative)

    :normal:
        :val1: The mean :math:`\mu` (may be positive or negative)
        :val2: The standard deviation :math:`\sigma` (must be positive)

    :lognormal:
        :val1: The mean of the underlying normal distribution :math:`\mu` (may be positive or negative)
        :val2: The standard deviation of the underlying normal distribution :math:`\sigma` (must be positive)
)pbdoc")
      .def(py::init<PvarsCell::Distribution, double, double>(),
           py::arg("dist") = PvarsCell::Distribution::none, py::arg("val1") = 0,
           py::arg("val2") = 0)
      .def_readwrite("dist", &PvarsCell::IonChanParam::dist,
                     "The distribution, which alters the meaning of the other members")
      .def_property(
          "val0",
          [](const PvarsCell::IonChanParam& param) { return param.val[0]; },
          [](PvarsCell::IonChanParam& param, double val) {
            param.val[0] = val;
          },
          R"pbdoc(Depends on distribution, see class description)pbdoc")
      .def_property(
          "val1",
          [](const PvarsCell::IonChanParam& param) { return param.val[1]; },
          [](PvarsCell::IonChanParam& param, double val) {
            param.val[1] = val;
          },R"pbdoc(Depends on distribution, see class description)pbdoc")
      .def("__repr__", IonChanParamREPR);

  py::class_<PvarsCurrentClamp, PvarsCell> pvarsCurr(m_Misc,
                                                     "PvarsCurrentClamp",
 R"pbdoc(Pvars specialized for the current clamp protocol)pbdoc");
  pvarsCurr.def(py::init<Protocol*>(), py::keep_alive<1, 2>())
      .def("__repr__", [pvarscell_REPR](PvarsCurrentClamp& c) {
        return "PvarsCurrentClamp(" + pvarscell_REPR(c) + ")";
      });
  py::class_<PvarsCurrentClamp::TIonChanParam, PvarsCell::IonChanParam>(
      pvarsCurr, "TIonChanParam",
R"pbdoc(IonChanParam specialized for the current clamp protocol, this subclass will be
  created automatically when an IonChanParam is assigned to a pvars)pbdoc")
      .def(py::init<>())
      .def_readwrite("trials", &PvarsCurrentClamp::TIonChanParam::trials,
                     "The cell parameter values for each trial")
      .def("__repr__", [IonChanParamREPR](PvarsCurrentClamp::TIonChanParam& p) {
        return "<TIonChanParam: " + IonChanParamREPR(p) + ">";
      });

  py::class_<PvarsVoltageClamp, PvarsCell> pvarsVolt(m_Misc,
                                                     "PvarsVoltageClamp",
R"pbdoc(Pvars specialized for the current clamp protocol)pbdoc");
  pvarsVolt.def(py::init<Protocol*>(), py::keep_alive<1, 2>())
      .def("__repr__", [pvarscell_REPR](PvarsVoltageClamp& c) {
        return "PvarsVoltageClamp(" + pvarscell_REPR(c) + ")";
      });
  py::class_<PvarsVoltageClamp::SIonChanParam, PvarsCell::IonChanParam>(
      pvarsVolt, "SIonChanParam",
R"pbdoc(IonChanParam specialized for the voltage clamp protocol, this subclass will be
  created automatically when an IonChanParam is assigned to a pvars)pbdoc")
      .def(py::init<>())
      .def_readwrite("val", &PvarsVoltageClamp::SIonChanParam::paramVal,
                     "The cell parameter value for the trial")
      .def("__repr__", [IonChanParamREPR](PvarsVoltageClamp::SIonChanParam& p) {
        return "<SIonChanParam: " + IonChanParamREPR(p) + ">";
      });

  py::class_<PvarsGrid, PvarsCell> pvarsGrid(m_Misc, "PvarsGrid",
R"pbdoc(Pvars specialized for the grid protocol)pbdoc");
  pvarsGrid.def(py::init<Grid*>(), py::keep_alive<1, 2>())
      .def("setMaxDistAndVal", &PvarsGrid::setMaxDistAndVal,
           "Set the maximum distance and the maximum value for a rule",
           py::arg("name"), py::arg("maxDist"), py::arg("maxVal"))
      .def("setStartCells", &PvarsGrid::setStartCells,
           "Set the cell locations in the grid that the rule will start from",
           py::arg("name"), py::arg("startCells"))
      .def("__repr__", [pvarscell_REPR](PvarsGrid& c) {
        return "PvarsGrid(" + pvarscell_REPR(c) + ")";
      });
  py::class_<PvarsGrid::MIonChanParam, PvarsCell::IonChanParam>(
      pvarsGrid, "MIonChanParam",
              R"pbdoc(IonChanParam specialized for the gird protocol, this subclass will be
                created automatically when an IonChanParam is assigned to a pvars)pbdoc")
      .def(py::init<>())
      .def_readwrite("maxDist", &PvarsGrid::MIonChanParam::maxDist,
                     "The maximum distance from the start that the rule will be applied")
      .def_readwrite("maxVal", &PvarsGrid::MIonChanParam::maxVal,
                     "The maxiumum value the rule will set the parameter for any cell to")
      .def_readwrite("startCells", &PvarsGrid::MIonChanParam::startCells,
                     "The starting cells. These are the first cells the rule is applied to, and the rule step is applied"
                     "outward until the maxiumum distance is obtained.")
      .def_readwrite("cells", &PvarsGrid::MIonChanParam::cells,
                     "The cells and the values they will be set to")
      .def("__repr__", [IonChanParamREPR](PvarsGrid::MIonChanParam& p) {
        return "<MIonChanParam: " + IonChanParamREPR(p) + ">";
      });

  py::class_<Measure>(m_Misc, "Measure",
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
      m_Misc, "MeasureDefault",
      R"pbdoc(Class that measures values for any cell variable that is not the transmembrane voltage (vOld))pbdoc")
      .def(py::init<>());
  py::class_<MeasureVoltage, Measure>(m_Misc, "MeasureVoltage",
      R"pbdoc(Class that measures values for the transmembrane voltage (vOld))pbdoc")
      .def(py::init<>())
      .def_property(
          "percrepol",
          (double (MeasureVoltage::*)(void) const) & MeasureVoltage::percrepol,
          (void (MeasureVoltage::*)(double)) & MeasureVoltage::percrepol,
              R"pbdoc(The percent repolarization at which an action potential ends)pbdoc");
  py::class_<MeasureFactory>(m_Misc, "MeasureFactory",
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

  py::class_<MeasureManager>(m_Misc, "MeasureManager",
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
  py::class_<GridMeasureManager, MeasureManager>(m_Misc, "GridMeasureManager",
        "The measure manager for grid simulations")
      .def(py::init<shared_ptr<GridCell>>())
      .def_property("dataNodes",
                    (set<pair<int, int>>(GridMeasureManager::*)(void)) &
                        GridMeasureManager::dataNodes,
                    (void (GridMeasureManager::*)(set<pair<int, int>>)) &
                        GridMeasureManager::dataNodes,
                    "The nodes on which measurements will be made");

}
