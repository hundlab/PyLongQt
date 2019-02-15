#include "protocol/currentClampProtocol.h"
#include "protocol/gridProtocol.h"
#include "protocol/protocol.h"
#include "protocol/voltageClampProtocol.h"
#include "pylongqt.h"
#include "settingsIO.h"
using namespace LongQt;
using namespace std;

void init_protocols(py::module& m) {
  py::module m_Protocols =
      m.def_submodule("Protocols",
                      "Contains all the Protocol classes. For easy use "
                      "protoMap provides constructors based on Protocol.type.");

  class Pars_view {
    Protocol* proto;

   public:
    Pars_view(Protocol* proto) { this->proto = proto; }
    bool has(std::string name) { return proto->hasPar(name); }
    std::string get(std::string name) { return proto->parsStr(name); }
    void set(std::string name, std::string val) { proto->parsStr(name, val); }
    std::list<std::pair<std::string, std::string>> allNames() {
      return proto->parsList();
    }
    std::string type(std::string name) { return proto->parsType(name); }
  };

  class Clamp_view {
    VoltageClamp* proto;

   public:
    Clamp_view(VoltageClamp* proto) { this->proto = proto; }
    std::pair<double, double> get(int pos) { return proto->clamps().at(pos); }
    int insert(std::pair<double, double> val) {
      return proto->insertClamp(val.first, val.second);
    }
    void setVoltage(int pos, double voltage) {
      proto->changeClampVoltage(pos, voltage);
    }
    void delVoltage(int pos) { proto->removeClamp(pos); }
    const std::vector<std::pair<double, double>> toList() {
      return proto->clamps();
    }
    void fromList(std::vector<std::pair<double, double>> clamps) {
      proto->clamps(clamps);
    }
  };

  py::class_<Pars_view>(m_Protocols, "_ParsVeiw",
                        "View for variables in Protocols")
      .def("__contains__", &Pars_view::has)
      .def("__getitem__", &Pars_view::get)
      .def("__setitem__", &Pars_view::set)
      .def("toList", &Pars_view::allNames)
      .def("getType", &Pars_view::type);

  py::class_<Clamp_view>(m_Protocols, "_ClampView",
                         "View for clamps in Voltage Clamp Protocol")
      .def("__getitem__", &Clamp_view::get)
      .def("insert", &Clamp_view::insert)
      .def("insert",
           [](Clamp_view& v, double time, double voltage) {
             v.insert({time, voltage});
           })
      .def("__delitem__", &Clamp_view::delVoltage)
      .def("setVoltage", &Clamp_view::setVoltage)
      .def("toList", &Clamp_view::toList)
      .def("fromList", &Clamp_view::fromList);

  py::class_<Protocol, shared_ptr<Protocol>>(
      m_Protocols, "Protocol",
      "Protocols determine what happens in the simulation. These classes "
      "contain configurations for the simulations.")
      .def("clone", &Protocol::clone)
      .def("runSim", &Protocol::runSim, "Run all the trials consecutively")
      .def_property("trial",
                    (unsigned int (Protocol::*)(void) const) & Protocol::trial,
                    (bool (Protocol::*)(unsigned int)) & Protocol::trial)
      .def("runTrial", &Protocol::runTrial, "Run the current trial")
      .def_property(
          "cell", (shared_ptr<Cell>(Protocol::*)(void) const) & Protocol::cell,
          (void (Protocol::*)(shared_ptr<Cell>)) & Protocol::cell)
      .def("setCellByName",
           (bool (Protocol::*)(const string&)) & Protocol::cell)
      .def("cellOptions", &Protocol::cellOptions,
           "possible cells that can be set with .cell")
      .def_property_readonly("pvars", &Protocol::pvars,
                             py::return_value_policy::reference_internal,
                             "The property settings. Used to modify the cells "
                             "constants at the beginning of each simulation.")
      .def_property_readonly(
          "measureMgr", &Protocol::measureMgr,
          py::return_value_policy::reference_internal,
          "The measurement settings. Tracks min,peak,etc of a cell variable")
      .def_property_readonly("type", &Protocol::type)
      .def_property("cellStateDir",
                    [](Protocol& p) { return p.parsStr("cellStateDir"); },
                    [](Protocol& p, string val) { p.parsStr("datadir", val); })
      .def_readwrite("cellStateFile", &Protocol::cellStateFile)
      .def_property("datadir", [](Protocol& p) { return p.parsStr("datadir"); },
                    [](Protocol& p, string val) { p.parsStr("datadir", val); })
      .def_readwrite("meastime", &Protocol::meastime)
      .def_readwrite("numtrials", &Protocol::numtrials)
      .def_readwrite("readCellState", &Protocol::readCellState)
      .def_readwrite("simvarfile", &Protocol::simvarfile)
      .def_readwrite("tMax", &Protocol::tMax)
      .def_readwrite("writeCellState", &Protocol::writeCellState)
      .def_readwrite("writeint", &Protocol::writeint)
      .def_readwrite("writetime", &Protocol::writetime)
      .def("setDataDir", &Protocol::setDataDir,
           "Set the directory where all data files will be written",
           py::arg("str") = "", py::arg("basedir") = "",
           py::arg("appendtxt") = "")
      .def("mkDirs", &Protocol::mkDirs)
      .def_readwrite("numruns", &Protocol::numruns,
                     "Number of times to run runDurring")
      .def_readwrite("runEvery", &Protocol::runEvery,
                     "Frequency to run runDurring")
      .def_readwrite("firstRun", &Protocol::firstRun,
                     "Time to start running runDurring")
      .def_property(
          "basedir", [](Protocol& p) { return p.basedir.absolutePath(); },
          [](Protocol& p, string path) { p.basedir.setPath(path.c_str()); })
      .def_property_readonly("pars", [](Protocol* p) { return Pars_view(p); });

  py::class_<CurrentClamp, Protocol, std::shared_ptr<CurrentClamp>>(
      m_Protocols, "CurrentClamp")
      .def(py::init<>())
      .def(py::pickle(
          [](shared_ptr<Protocol> p) {
            QString str;
            SettingsIO::getInstance()->writeSettingsStr(&str, p);
            return str.toStdString();
          },
          [](const string& s) {
            QString str(s.c_str());
            auto p = CellUtils::protoMap.at("Current Clamp Protocol")();
            auto set = SettingsIO::getInstance();
            set->readSettingsStr(str, p);
            return *dynamic_pointer_cast<CurrentClamp>(p);
          }))
      .def_readwrite("bcl", &CurrentClamp::bcl)
      .def_readwrite("paceflag", &CurrentClamp::paceflag)
      .def_readwrite("numstims", &CurrentClamp::numstims)
      .def_readwrite("stimdur", &CurrentClamp::stimdur)
      .def_readwrite("stimval", &CurrentClamp::stimval)
      .def_readwrite("stimt", &CurrentClamp::stimt);
  py::class_<VoltageClamp, Protocol, std::shared_ptr<VoltageClamp>>(
      m_Protocols, "VoltageClamp")
      .def(py::init<>())
      .def(py::pickle(
          [](shared_ptr<Protocol> p) {
            QString str;
            SettingsIO::getInstance()->writeSettingsStr(&str, p);
            return str.toStdString();
          },
          [](const string& s) {
            QString str(s.c_str());
            auto p = CellUtils::protoMap.at("Voltage Clamp Protocol")();
            auto set = SettingsIO::getInstance();
            set->readSettingsStr(str, p);
            return *dynamic_pointer_cast<VoltageClamp>(p);
          }))
      .def_property_readonly("clamps",
                             [](VoltageClamp* p) { return Clamp_view(p); });

  py::class_<GridProtocol, CurrentClamp, std::shared_ptr<GridProtocol>>(
      m_Protocols, "GridProtocol")
      .def(py::init<>())
      .def(py::pickle(
          [](shared_ptr<Protocol> p) {
            QString str;
            SettingsIO::getInstance()->writeSettingsStr(&str, p);
            return str.toStdString();
          },
          [](const string& s) {
            QString str(s.c_str());
            auto p = CellUtils::protoMap.at("Grid Protocol")();
            auto set = SettingsIO::getInstance();
            set->readSettingsStr(str, p);
            return *dynamic_pointer_cast<GridProtocol>(p);
          }))
      .def_property_readonly("grid", &GridProtocol::getGrid)
      .def_property_readonly("stimNodes", &GridProtocol::getStimNodes,
                             "nodes to stimulate")
      .def_property("stim2", &GridProtocol::getStim2, &GridProtocol::setStim2,
                    "enable a second stimulus")
      .def_readwrite("stimNodes", &GridProtocol::stimNodes)
      .def_readwrite("stimNodes2", &GridProtocol::stimNodes2)
      .def_readwrite("stimval2", &GridProtocol::stimval2)
      .def_readwrite("stimdur2", &GridProtocol::stimdur2)
      .def_readwrite("bcl2", &GridProtocol::bcl2)
      .def_readwrite("stimt2", &GridProtocol::stimt2);
}
