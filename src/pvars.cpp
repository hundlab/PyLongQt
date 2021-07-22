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

void init_pvars(py::module& m) {
  py::module m_Pvar = m.def_submodule("CellParameters",
                                      "Configure the cell parameters for each trial");

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
  py::class_<PvarsCell> pvarscell(m_Pvar, "PvarsCell",
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

  py::class_<PvarsCurrentClamp, PvarsCell> pvarsCurr(m_Pvar,
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

  py::class_<PvarsVoltageClamp, PvarsCell> pvarsVolt(m_Pvar,
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

  py::class_<PvarsGrid, PvarsCell> pvarsGrid(m_Pvar, "PvarsGrid",
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
}
