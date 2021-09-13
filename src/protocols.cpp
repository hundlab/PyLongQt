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
                      R"pbdoc(
                      Protocols control what happens to the cell(s) throughout the simulation. There are currently 3 types of protocols:

                      1. Current Clamp: stimulates the cell at a given frequency
                      2. Voltage Clamp: fixes the transmembrane voltage for specific lengths of time
                      3. Grid: for 1D fiber, and 2D grid simulations stimulating as in Current Clamp
                      )pbdoc");

  class Pars_view {
    Protocol* proto;

   public:
    std::list<std::pair<std::string, std::string>> allNames;

    Pars_view(Protocol* proto) {
      this->proto = proto;
      this->allNames = proto->parsList();
    }
    bool has(std::string name) { return proto->hasPar(name); }
    std::string get(std::string name) { return proto->parsStr(name); }
    void set(std::string name, std::string val) { proto->parsStr(name, val); }
    std::string type(std::string name) { return proto->parsType(name); }
  };

  class Clamp_view {
    VoltageClamp* proto;

   public:
    std::vector<std::pair<double, double>> clampsList;

    Clamp_view(VoltageClamp* proto) {
      this->proto = proto;
      this->clampsList = proto->clamps();
    }
    std::pair<double, double> get(int pos) { return proto->clamps().at(pos); }
    int insert(std::pair<double, double> val) {
      return proto->insertClamp(val.first, val.second);
    }
    void setVoltage(int pos, double voltage) {
      proto->changeClampVoltage(pos, voltage);
    }
    void delVoltage(int pos) { proto->removeClamp(pos); }
    void fromList(std::vector<std::pair<double, double>> clamps) {
      proto->clamps(clamps);
    }
  };

  py::class_<Pars_view>(m_Protocols, "ParsVeiw",
                        "Interface for working with Protocol variables. Accessable through pars property in each Protocol.")
      .def("__contains__", &Pars_view::has,
           R"pubdoc(Checks if the protocol has that parameter
           :name: The name of the parameter)pubdoc", py::arg("name"))
      .def("__getitem__", &Pars_view::get,
           R"pubdoc(Retrieves the parameter's value
           :name: The name of the parameter)pubdoc", py::arg("name"))
      .def("__setitem__", &Pars_view::set,
           R"pubdoc(Sets the parameter to value
           :name: The name of the parameter
           :value: The new value)pubdoc", py::arg("name"), py::arg("value"))
      .def("__iter__",
           [](Pars_view& v) { return py::make_iterator(v.allNames); },
            R"pubdoc(Iterate over parameters)pubdoc",
           py::keep_alive<0, 1>())
      .def("getType", &Pars_view::type,
           R"pubdoc(Retrieves the parameter's class type
           :name: The name of the parameter)pubdoc",
           py::arg("name"));

  py::class_<Clamp_view>(m_Protocols, "ClampView",
                         "Interface for voltages clamps. Access through clamps in Voltage Clamp Protocol")
      .def("__getitem__", &Clamp_view::get,
           R"pubdoc(Retrieves the clamps start time (ms) and voltage (mV)
           :index: That clamp's index in the ordered list of clamps)pubdoc", py::arg("index"))
      .def("insert", &Clamp_view::insert,
           R"pubdoc(Insert a new clamp into the list of clamps
           :pair: The clamp's start time (ms) and voltage (mV))pubdoc",
           py::arg("pair"))
      .def("insert",
           [](Clamp_view& v, double time, double voltage) {
             v.insert({time, voltage});
           },
          R"pubdoc(Insert a new clamp into the list of clamps
          :time: The clamp's start time (ms)
          :voltage: The clamp's voltage (mV))pubdoc",
           py::arg("time"), py::arg("voltage"))
      .def("__delitem__", &Clamp_view::delVoltage,
           R"pubdoc(Delete a clamp from the list of clamps
           :index: That clamp's index in the ordered list of clamps)pubdoc",
           py::arg("index"))
      .def("setVoltage", &Clamp_view::setVoltage,
           R"pubdoc(Change the voltage of a clamp
           :index: That clamp's index in the ordered list of clamps
           :voltage: The clamp's voltage (mV))pubdoc",
           py::arg("index"),
           py::arg("voltage"))
      .def("__iter__",
           [](Clamp_view& v) { return py::make_iterator(v.clampsList); },
           R"pubdoc(Iterate over clamps)pubdoc",
           py::keep_alive<0, 1>())
      .def("__repr__",
	   [](Clamp_view& v) {
	   	return py::print(v.clampsList); })	
      .def("fromList", &Clamp_view::fromList,
           R"pubdoc(Import clamps from a list)pubdoc",
           py::arg("voltageList"));

  py::class_<Protocol, shared_ptr<Protocol>>(
      m_Protocols, "Protocol",
              R"pubdoc(The base class of all protocols)pubdoc")
      .def("clone", &Protocol::clone, R"pubdoc(Create a copy/clone of the protocol)pubdoc")
      .def("runSim", &Protocol::runSim,
           R"pubdoc(Run all the trials consecutively.
           .. Note: it is recommended to use Misc.RunSim instead)pubdoc")
      .def_property("trial",
                    (int (Protocol::*)(void) const) & Protocol::trial,
                    (bool (Protocol::*)(int)) & Protocol::trial,
                    R"pubdoc(The current trial number. Different trials with have cell parameters. See pvars for more detial.)pubdoc")
      .def("runTrial", &Protocol::runTrial, "Run the current trial")
      .def_property(
          "cell", (shared_ptr<Cell>(Protocol::*)(void) const) & Protocol::cell,
          (void (Protocol::*)(shared_ptr<Cell>)) & Protocol::cell,
              R"pubdoc(The cell model used in the simulation)pubdoc")
      .def("setCellByName",
           (bool (Protocol::*)(const string&)) & Protocol::cell,
           R"pubdoc(Set the cell using its name rather than a cell model object
           :name: The name of the cell model)pubdoc",
           py::arg("cellName"))
      .def("cellOptions", &Protocol::cellOptions,
           R"pubdoc(The names of the cell models which this protocol can take
           .. Note: this may differ from the cellMap in the top-level namespace,
                    as some protocols restrict the possible cell models)pubdoc")
      .def_property_readonly("pvars", &Protocol::pvars,
                             py::return_value_policy::reference_internal,
                             R"pubdoc(The cell's properties. These are cell model constants which are not changed throughout
                             the simulation, but may be different from one trial (simulation) to annother)pubdoc")
      .def_property_readonly(
          "measureMgr", &Protocol::measureMgr,
          py::return_value_policy::reference_internal,
              R"pubdoc(The manager for cell's measured variables. Many of the cell model's internal variables can be tracked
              throughout the simulation and functions of those variables can be recored, such as the peak, min, etc. The
              advantage to using this interface rather than computing these measures post simulation is that within LongQt
              they can be computed using every time step, while usually only a fraction of the timesteps are saved. This
              reduces the amount of disk space and computational time needed.)pubdoc")
      .def_property_readonly("type", &Protocol::type,
                             R"pubdoc(The name of the protocol class)pubdoc")
      .def_property("cellStateDir",
                    [](Protocol& p) { return p.parsStr("cellStateDir"); },
                    [](Protocol& p, string val) { p.parsStr("datadir", val); },
        R"pubdoc(The directory to read/write the complete state of the cell model.
        .. Note: this is an expiremental feature and may not work correctly for every cell)pubdoc")
      .def_readwrite("cellStateFile", &Protocol::cellStateFile,
                     R"pubdoc(The file name to of the cell state. See cellStateDir.)pubdoc")
      .def_property("datadir", [](Protocol& p) { return p.parsStr("datadir"); },
                    [](Protocol& p, string val) { p.parsStr("datadir", val); },
        R"pubdoc(The directory where the results of the simulations will be saved.)pubdoc")
      .def_readwrite("meastime", &Protocol::meastime,
                     R"pubdoc(The time (ms) when the measure manager will begin measuring.)pubdoc")
      .def_property("numtrials", (int(Protocol::*)(void) const) &Protocol::numtrials,
          (void(Protocol::*)(int)) &Protocol::numtrials,
                    R"pubdoc(The totoal number of trials (simulations) to be run.)pubdoc")
      .def_readwrite("readCellState", &Protocol::readCellState,
                     "Whether the cell state should be read")
      .def_readwrite("simvarfile", &Protocol::simvarfile,
                     R"pubdoc(The file where the simulation settings will be written. The settings include the protocol configuration,
                     the cell, the cell's properties, the measures, as well as the structure: in short almost everything. The
                     directory will be datadir.)pubdoc")
      .def_readwrite("tMax", &Protocol::tMax, R"pubdoc(The maxium length of each simulation (ms))pubdoc")
      .def_readwrite("writeCellState", &Protocol::writeCellState, R"pubdoc(Whether the cell state should be written)pubdoc")
      .def_readwrite("writeint", &Protocol::writeint, R"pubdoc(How often the cell's variable traces should be recored. Every writeint
                                                      number of simulation timesteps the traces will be recorded.)pubdoc")
      .def_readwrite("writetime", &Protocol::writetime, R"pubdoc(The time (ms) when traces will begin being recorded)pubdoc")
      .def("setDataDir", &Protocol::setDataDir,
           R"pubdoc(Set the directory where the results of the simulations will be saved.
           datadir = basedir + name + datetime + appendtxt + (optional number for uniqueness)
           :name: the name of the datadir, if this is omitted the standard naming scheme will be used
           :basedir: the directory where the datadir will be created, if omited basedir will be used (the basedir property will
           be set)
           :appendtxt: additional text to be appended to the end
           :append_date: if datetime will be appended after name)pubdoc",
           py::arg("name") = "/data",
           py::arg("basedir") = "",
           py::arg("appendtxt") = "",
           py::arg("append_date") = true)
      .def("mkDirs", &Protocol::mkDirs, R"pubdoc(Create datadir (will be called automatically when simulations are run))pubdoc")
      .def("setRunDuring", [] (Protocol& p, std::function<void(Protocol*)> func, double firstRun = -1,
           double runEvery = -1, int numruns = -1) {
            auto new_func = [func] (Protocol& p) { func(&p); };
            p.setRunDuring(new_func, firstRun, runEvery, numruns);},
           py::arg("func"), py::arg("firstRun") = -1,
           py::arg("runEvery") = -1, py::arg("numruns") = -1,
           "This takes a function which will"
            " be called durring the simulation"
           " at time firstRun, and then every runEvery"
           " after that until the end of the simulation"
           " or func has been run numruns number of times."
            "The function must take a the "
            "protocol class which will be run "
            "as an arguement, and it should not "
            "return anything.")
      .def("setRunBefore", [] (Protocol& p, std::function<void(Protocol*)> func) {
            auto new_func = [func] (Protocol& p) { func(&p); };
            p.setRunBefore(new_func);},
           py::arg("func"),
           "This takes a function which will"
            " be called before the simulation"
            " is started but after the"
            " simulation has been setup. "
            "The function must take a the "
            "protocol class which will be run "
            "as an arguement, and it should not "
            "return anything.")
      .def("setRunAfter", [] (Protocol& p, std::function<void(Protocol*)> func) {
               auto new_func = [func] (Protocol& p) { func(&p); };
               p.setRunBefore(new_func);},
           py::arg("func"),
           "This takes a function which will"
            " be called after the simulation"
            " is finished. "
            "The function must take a the "
            "protocol class which will be run "
            "as an arguement, and it should not "
            "return anything.")
      .def_readwrite("numruns", &Protocol::numruns,
                     "Number of times to run runDurring")
      .def_readwrite("runEvery", &Protocol::runEvery,
                     "Frequency to run runDurring")
      .def_readwrite("firstRun", &Protocol::firstRun,
                     "Time to start running runDurring")
      .def_property(
          "basedir", [](Protocol& p) { return p.basedir.absolutePath().toStdString(); },
          [](Protocol& p, string path) { p.basedir.setPath(path.c_str()); },
           "The directory in which the datadir will be created")
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
      .def_readwrite("bcl", &CurrentClamp::bcl, "The basic cycle length (ms). After stimt the cell will be stimulated every "
                                                "bcl miliseconds.")
      .def_readwrite("paceflag", &CurrentClamp::paceflag, "If the cell should be stimulated. When this is falce no other stimulus "
                                                          "related parameters have an effect")
      .def_readwrite("numstims", &CurrentClamp::numstims, "The total number of times the stimulus should be applied in the simulation")
      .def_readwrite("stimdur", &CurrentClamp::stimdur, "The duration of each stimulus (ms)")
      .def_readwrite("stimval", &CurrentClamp::stimval, "The current at which the cell should be stimulated")
      .def_readwrite("stimt", &CurrentClamp::stimt, "The time (ms) at which the first stimulus should be applied");
  py::class_<VoltageClamp, Protocol, std::shared_ptr<VoltageClamp>>(
      m_Protocols, "VoltageClamp",
              R"pubdoc(This protocol fixes the cell model's transmembrane voltage to different values throughout the course of
              the simulation as defined by the clamp list)pubdoc")
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
                             [](VoltageClamp* p) { return Clamp_view(p); },
        R"pubdoc(The list of clamps which fix the cells transmembrane voltage to a specfic value (mv) starting at a time (ms). The
        pairs in the list are (time, voltage), and are ordered by time.)pubdoc");

  py::class_<GridProtocol, CurrentClamp, std::shared_ptr<GridProtocol>>(
      m_Protocols, "GridProtocol", R"pubdoc(This protocol is for 1D fibers or 2D grids of cells. It allows for current clamp like"
                                   "simulations with the groups of cells rather than just individual cells as in Voltage Clamp and"
                                   "Current Clamp.)pubdoc")
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
      .def_property_readonly("grid", &GridProtocol::getGrid,
                             "The structure of cells, if this is 1D then it is a fibre simulation, if it's 2D then it's a grid"
                             " simulation.")
      .def_readwrite("stimNodes", &GridProtocol::stimNodes,
                     "The cells which the current stimulus will be applied to. The cells not on this list will not be excited "
                     "directly but may excited by their neighbors.");
}
