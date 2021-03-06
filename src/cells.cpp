#include "cell/FR.h"
#include "cell/OHaraRudy.h"
#include "cell/OHaraRudyEndo.h"
#include "cell/OHaraRudyEpi.h"
#include "cell/OHaraRudyM.h"
#include "cell/atrial.h"
#include "cell/br04.h"
#include "cell/cell.h"
#include "cell/cell_kernel.h"
#include "cell/coupledinexcitablecell.h"
#include "cell/gpbatrial.h"
#include "cell/gpbatrialonal17.h"
#include "cell/gpbhuman.h"
#include "cell/gridCell.h"
#include "cell/hrd09.h"
#include "cell/inexcitablecell.h"
#include "cell/ksan.h"
#include "cell/kurata08.h"
#include "cell/tnnp04.h"
#include "cellutils.h"
#include "pylongqt.h"
using namespace LongQt;

void init_cells(py::module& m) {
  py::module m_Cells =
      m.def_submodule("Cells",
                      "Contains all the Cell classes. For easy use cellMap "
                      "provides constructors based on Cell.type.");

  class Cellars_view {
    CellKernel* cell;

   public:
    std::set<std::string> namesSet;
    enum type { vars = 0, pars = 1 } ars_type;
    Cellars_view(CellKernel* cell, type type) {
      this->cell = cell;
      this->ars_type = type;
      namesSet = ars_type == vars ? cell->vars() : cell->pars();
    }
    bool has(std::string name) {
      return ars_type == vars ? cell->hasVar(name) : cell->hasPar(name);
    }
    double get(std::string name) {
      return ars_type == vars ? cell->var(name) : cell->par(name);
    }
    void set(std::string name, double val) {
      ars_type == vars ? cell->setVar(name, val) : cell->setPar(name, val);
    }
    std::set<std::string> allNames() { return namesSet; }
    std::string toString() {
      std::string ret = "{";
      for (auto& name : namesSet) {
        ret += "'" + name + "', ";
      }
      ret += "}";
      return ret;
    }
    int length() { return namesSet.size(); }
  };

  py::class_<Cellars_view>(m_Cells, "_VarsParsVeiw",
                           "View for variables and constants in Cell's")
      .def("__contains__", &Cellars_view::has)
      .def("__getitem__", &Cellars_view::get)
      .def("__setitem__", &Cellars_view::set)
      .def("__str__", &Cellars_view::toString)
      .def("__repr__",
           [](Cellars_view&) { return "PyLongQt.Cells._VarsParsVeiw"; })
      .def("__iter__",
           [](Cellars_view& v) { return py::make_iterator(v.namesSet); },
           py::keep_alive<0, 1>())
      .def("keys", &Cellars_view::allNames);

  py::class_<CellKernel, std::shared_ptr<CellKernel>>(m_Cells, "CellKernel",
                                                      "Base class of all cells")
      .def("clone", &CellKernel::clone)
      .def("updateV", &CellKernel::updateV, "Update voltages")
      .def("setV", &CellKernel::setV, "Set total voltage", py::arg("voltage"))
      .def("updateCurr", &CellKernel::updateCurr, "Update currents")
      .def("updateConc", &CellKernel::updateConc, "Update concentrations")
      .def("tstep", &CellKernel::tstep, "Increment the time used by the cell",
           py::arg("stimulus time"))
      .def("externalStim", &CellKernel::externalStim, "Stimulate the cell",
           py::arg("stimulus"))
      .def_readwrite("vOld", &CellKernel::vOld)
      .def_readwrite("t", &CellKernel::t)
      .def_readwrite("dt", &CellKernel::dt)
      .def_readwrite("iNat", &CellKernel::iNat)
      .def_readwrite("iKt", &CellKernel::iKt)
      .def_readwrite("iCat", &CellKernel::iCat)
      .def_readwrite("iTot", &CellKernel::iTot)
      .def_readwrite("iTotold", &CellKernel::iTotold)
      .def_readwrite("Cm", &CellKernel::Cm)
      .def_readwrite("Vcell", &CellKernel::Vcell)
      .def_readwrite("Vmyo", &CellKernel::Vmyo)
      .def_readwrite("AGeo", &CellKernel::AGeo)
      .def_readwrite("ACap", &CellKernel::ACap)
      .def_readwrite("Rcg", &CellKernel::Rcg)
      .def_readwrite("cellRadius", &CellKernel::cellRadius)
      .def_readwrite("cellLength", &CellKernel::cellLength)
      .def_readwrite("dVdt", &CellKernel::dVdt)
      .def_readwrite("Rmyo", &CellKernel::Rmyo)
      .def_readwrite("dtmin", &CellKernel::dtmin)
      .def_readwrite("dtmed", &CellKernel::dtmed)
      .def_readwrite("dtmax", &CellKernel::dtmax)
      .def_readwrite("dvcut", &CellKernel::dvcut)
      .def_readwrite("apTime", &CellKernel::apTime)
      .def_readwrite("RGAS", &CellKernel::RGAS)
      .def_readwrite("TEMP", &CellKernel::TEMP)
      .def_readwrite("FDAY", &CellKernel::FDAY)
      .def_property_readonly("vars",
                             [](CellKernel* cell) {
                               return Cellars_view(cell, Cellars_view::vars);
                             })
      .def_property_readonly("pars",
                             [](CellKernel* cell) {
                               return Cellars_view(cell, Cellars_view::pars);
                             })
      .def_property_readonly("type", &CellKernel::type)
      .def("optionsMap", &CellKernel::optionsMap)
      .def("setOption", &CellKernel::setOption, py::arg("name"),
           py::arg("value"))
      .def("getOption", &CellKernel::option, py::arg{"name"});

  py::class_<Cell, std::shared_ptr<Cell>, CellKernel>(
      m_Cells, "Cell",
      "Base class for cells. Provides selections of variables & constants to "
      "be written during the simulation")
      //            .def(py::init<const std::string &>())
      .def_property("constantSelection", &Cell::getConstantSelection,
                    &Cell::setConstantSelection)
      .def_property("variableSelection", &Cell::getVariableSelection,
                    &Cell::setVariableSelection)
      .def("__repr__", [](Cell& c) {
        return "<Cell Type='" + std::string(c.type()) + "'>";
      });

  py::class_<Kurata08, std::shared_ptr<Kurata08>, Cell>(m_Cells, "Kurata08")
      .def(py::init<>());
  py::class_<OHaraRudy, std::shared_ptr<OHaraRudy>, Cell>(m_Cells, "OHaraRudy")
      .def(py::init<>());
  py::class_<OHaraRudyEndo, std::shared_ptr<OHaraRudyEndo>, OHaraRudy>(
      m_Cells, "OHaraRudyEndo")
      .def(py::init<>());
  py::class_<OHaraRudyEpi, std::shared_ptr<OHaraRudyEpi>, OHaraRudy>(
      m_Cells, "OHaraRudyEpi")
      .def(py::init<>());
  py::class_<OHaraRudyM, std::shared_ptr<OHaraRudyM>, OHaraRudy>(m_Cells,
                                                                 "OHaraRudyM")
      .def(py::init<>());
  py::class_<Courtemanche98, std::shared_ptr<Courtemanche98>, Cell>(
      m_Cells, "Courtemanche98")
      .def(py::init<>());
  py::class_<Br04, std::shared_ptr<Br04>, Cell>(m_Cells, "Br04")
      .def(py::init<>());
  py::class_<GpbAtrial, std::shared_ptr<GpbAtrial>, Cell>(m_Cells, "GpbAtrial")
      .def(py::init<>());
  py::class_<GpbVent, std::shared_ptr<GpbVent>, Cell>(m_Cells, "GpbVent")
      .def(py::init<>());
  py::class_<HRD09Control, std::shared_ptr<HRD09Control>, Cell>(m_Cells,
                                                                "HRD09Control")
      .def(py::init<>());
  py::class_<HRD09BorderZone, std::shared_ptr<HRD09BorderZone>, HRD09Control>(
      m_Cells, "HRD09BorderZone")
      .def(py::init<>());
  py::class_<InexcitableCell, std::shared_ptr<InexcitableCell>, Cell>(
      m_Cells, "InexcitableCell")
      .def(py::init<>());
  py::class_<CoupledInexcitableCell, std::shared_ptr<CoupledInexcitableCell>,
             Cell>(m_Cells, "CoupledInexcitableCell")
      .def(py::init<>());
  py::class_<Ksan, std::shared_ptr<Ksan>, Cell>(m_Cells, "Ksan")
      .def(py::init<>());
  py::class_<TNNP04Control, std::shared_ptr<TNNP04Control>, Cell>(
      m_Cells, "TNNP04Control")
      .def(py::init<>());
  py::class_<FR, std::shared_ptr<FR>, Cell>(m_Cells, "FaberRudy")
      .def(py::init<>());

  class PyGpbAtrialOnal17: public GpbAtrialOnal17 {
  public:
      using GpbAtrialOnal17::GpbAtrialOnal17;
      PyGpbAtrialOnal17* clone() override {PYBIND11_OVERLOAD(PyGpbAtrialOnal17*, PyGpbAtrialOnal17, clone, );}
      void setup() override {PYBIND11_OVERLOAD(void, PyGpbAtrialOnal17, setup, );}
      const char* type() const override {PYBIND11_OVERLOAD_NAME(const char*, GpbAtrialOnal17, "cell_type", type, );}
      void updateIna() override {PYBIND11_OVERLOAD(void, GpbAtrialOnal17, updateIna, );}
  };
  py::class_<GpbAtrialOnal17, PyGpbAtrialOnal17, std::shared_ptr<GpbAtrialOnal17>, Cell>(
      m_Cells, "GpbAtrialOnal17")
      .def(py::init<>())
      .def("updateIna", &GpbAtrialOnal17::updateIna);

  py::class_<GridCell, std::shared_ptr<GridCell>, Cell>(m_Cells, "GridCell")
      .def(py::init<>())
// windows compiler static issues
#if !defined(_WIN32) && !defined(_WIN64)
      .def_property_readonly("grid", &GridCell::getGrid)
#endif
      ;
}
