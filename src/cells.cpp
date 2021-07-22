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
                      R"pbdoc(
                      Cells contains the cell objects which encompass mathematicals and code of the electrophysiological cardiomyocycte cell
                      models. Each model is derived from a publication where the mathmatical equations are presented along with the
                      results and validation for the model. Typically, the easiest way to create a cell object is through the
                      :py:data:`PyLongQt.cellMap` dictionary, or through :py:meth:`Protocol.setCellByName`.
                      )pbdoc");

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

  py::class_<Cellars_view>(m_Cells, "VarsParsVeiw",
                           R"pbdoc(
                           Interface for accessing and setting a cell's variables or constants. Variables are values in a cell
                           which change durring a simulation, while constants will not. Some constants may be manipulated through
                           :py:data:`Protocol.pvars`)pbdoc")
      .def("__contains__", &Cellars_view::has,
           R"pbdoc(
           Checks if cell has that variable/constant.
           :name: The name of the variable/constant to check
           )pbdoc", py::arg("name"))
      .def("__getitem__", &Cellars_view::get,
           R"pbdoc(
            Gets the value of a variable/constant.
            :name: The name of the variable/constant
            )pbdoc", py::arg("name"))
      .def("__setitem__", &Cellars_view::set,
           R"pbdoc(
            Sets the value of a variable/constant to a value.
            :name: The name of the variable/constant
            :value: The value to set the variable/constant to
            )pbdoc", py::arg("name"), py::arg("value"))
      .def("__str__", &Cellars_view::toString)
      .def("__repr__",
           [](Cellars_view&) { return "PyLongQt.Cells._VarsParsVeiw"; })
      .def("__iter__",
           [](Cellars_view& v) { return py::make_iterator(v.namesSet); },
           R"pubdoc(Iterate over variables/constants)pubdoc",
           py::keep_alive<0, 1>())
      .def("keys", &Cellars_view::allNames,
           R"pubdoc(Get the names of all variables/constants)pubdoc");

  py::class_<CellKernel, std::shared_ptr<CellKernel>>(m_Cells, "CellKernel",
          R"pubdoc(Base class for all cell objects. This class provides the interface for interacting and manipulating cells.
          Further, the :py:class:`Cell` class adds additional functionallity to cells relevant to i/o)pubdoc")
      .def("clone", &CellKernel::clone, R"pubdoc(Create a copy/clone of the cell)pubdoc")
      .def("updateV", &CellKernel::updateV, "Update the cell's transmembrane voltage.")
      .def("setV", &CellKernel::setV, "Set transmembrane voltage",
           py::arg("voltage"))
      .def("updateCurr", &CellKernel::updateCurr, "Update the cell's ion channel and flux currents")
      .def("updateConc", &CellKernel::updateConc, "Update the cell's ion concentrations and signaling molecules")
      .def("tstep", &CellKernel::tstep, R"pbdoc(Increment the cell's time by :py:data:`dt`.
                                        :stim_time: the time (ms) when the next stimulus will occur.
                                                    This is used to determine the appropriate timestep size.)pbdoc",
           py::arg("stim_time"))
      .def("externalStim", &CellKernel::externalStim, R"pbdoc(
Apply a stimulus to the cell
:stimulus: The current (pA/pF) to apply to the cell. Will be applied for only
the timestep when it is called.)pbdoc",
           py::arg("stimulus"))
      .def_readwrite("vOld", &CellKernel::vOld, "The transmembrane voltage (mV)")
      .def_readwrite("t", &CellKernel::t, "The current time (ms)")
      .def_readwrite("dt", &CellKernel::dt, "The current timestep (ms)")
      .def_readwrite("iNat", &CellKernel::iNat, "Total sodium current (pA/pF)")
      .def_readwrite("iKt", &CellKernel::iKt, "The total potassium current (pA/pF)")
      .def_readwrite("iCat", &CellKernel::iCat, "The total calcium current (pA/pF)")
      .def_readwrite("iTot", &CellKernel::iTot, "The total current (pA/pF)")
      .def_readwrite("iTotold", &CellKernel::iTotold, "The total current from the previous timestep")
      .def_readwrite("Cm", &CellKernel::Cm, "Cell capacitance (uF/cm^2)")
      .def_readwrite("Vcell", &CellKernel::Vcell, "Cell volume ()")
      .def_readwrite("Vmyo", &CellKernel::Vmyo, "Cell Myoplasmic volume (uL)")
      .def_readwrite("AGeo", &CellKernel::AGeo, "Geometric cell surface area")
      .def_readwrite("ACap", &CellKernel::ACap, "Capacitive cell surface area")
      .def_readwrite("Rcg", &CellKernel::Rcg, "Ratio of capacitive to geometric area")
      .def_readwrite("cellRadius", &CellKernel::cellRadius, "Cell radius (cm). Cells are modeled as cylinders.")
      .def_readwrite("cellLength", &CellKernel::cellLength, "Cell length (cm). Cells are modeled as cylinders.")
      .def_readwrite("dVdt", &CellKernel::dVdt, "Change in voltage over change in time from the last timestep")
      .def_readwrite("Rmyo", &CellKernel::Rmyo, "Myoplasmic resistivity")
      .def_readwrite("dtmin", &CellKernel::dtmin, "Minimum timestep size (ms)")
      .def_readwrite("dtmed", &CellKernel::dtmed, "Medium timestep size (ms)")
      .def_readwrite("dtmax", &CellKernel::dtmax, "Maximum timestep size (ms)")
      .def_readwrite("dvcut", &CellKernel::dvcut, R"pbdoc(What is this(mV/ms))pbdoc")
      .def_readwrite("apTime", &CellKernel::apTime, "What is this")
      .def_readwrite("RGAS", &CellKernel::RGAS, "R gass constant")
      .def_readwrite("TEMP", &CellKernel::TEMP, "Cell temperature (K)")
      .def_readwrite("FDAY", &CellKernel::FDAY, "Faraday's constant (C/mol)")
      .def_property_readonly("vars",
                             [](CellKernel* cell) {
                               return Cellars_view(cell, Cellars_view::vars);
                             },
        R"pbdoc(The variables for the cell. Variables are values which will change throughout the simulation.
        Different cell models will have different variables.)pbdoc")
      .def_property_readonly("pars",
                             [](CellKernel* cell) {
                               return Cellars_view(cell, Cellars_view::pars);
                             },
        R"pbdoc(The constants for the cell. Constants are values which will
        not change throughout the simulation. Some of these values
        can be set using :py:data:`PyLongQt.Protocols.Protocol.pvars`.
        Different cell models will have different constants.)pbdoc")
      .def_property_readonly("type", &CellKernel::type, "The name of the cell model")
      .def("optionsMap", &CellKernel::optionsMap,
           R"pbdoc(Options which modify the cell model. The options map contains all possible options
           and their statuses.
           ..Note: Some options may be mutually exclusive.)pbdoc")
      .def("setOption", &CellKernel::setOption,
           R"pbdoc(Set one of the cells options.
           :name: The name of the option
           :value: Whether the option should be true or false)pbdoc",
           py::arg("name"), py::arg("value"))
      .def("getOption", &CellKernel::option,
           R"pbdoc(Get the status of one of the cells options.
           :name: The name of the option)pbdoc",
           py::arg{"name"});

  py::class_<Cell, std::shared_ptr<Cell>, CellKernel>(
      m_Cells, "Cell",
      R"pbdoc(Base class for cells built on py:class:`CellKernel`. Provides selections of variables & constants to
      be tracked and saved during the simulation)pbdoc")
      //            .def(py::init<const std::string &>())
      .def_property("constantSelection", &Cell::getConstantSelection,
                    &Cell::setConstantSelection,
                    R"pbdoc(The selection of constants to be tracked during the simulation)pbdoc")
      .def_property("variableSelection", &Cell::getVariableSelection,
                    &Cell::setVariableSelection,
                    R"pbdoc(The selection of variables to be tracked during the simulation)pbdoc")
      .def("__repr__", [](Cell& c) {
        return "<Cell Type='" + std::string(c.type()) + "'>";
      });

  py::class_<Kurata08, std::shared_ptr<Kurata08>, Cell>(m_Cells, "Kurata08",
      R"pbdoc(
Rabbit Sinus Node (Kurata 2008)

| Kurata, Yasutaka, et al. “Regional Difference in Dynamical Property of
|    Sinoatrial Node Pacemaking: Role of Na+channel Current.” Biophysical Journal,
|    vol. 95, no. 2, 2008, pp. 951–77, doi:10.1529/biophysj.107.112854.)pbdoc")
      .def(py::init<>());
  py::class_<OHaraRudy, std::shared_ptr<OHaraRudy>, Cell>(m_Cells, "OHaraRudy")
      .def(py::init<>());
  py::class_<OHaraRudyEndo, std::shared_ptr<OHaraRudyEndo>, OHaraRudy>(
      m_Cells, "OHaraRudyEndo",
              R"pbdoc(
Human Ventricular Endocardium (O'Hara-Rudy 2011)

| O’hara, Thomas, et al. “Simulation of the Undiseased Human Cardiac Ventricular
|    Action Potential: Model Formulation and Experimental Validation.” PLoS Comput
|    Biol, vol. 7, no. 5, American Heart Association, 2011, pp. 1002061–302,
|    doi:10.1371/journal.pcbi.1002061.)pbdoc")
      .def(py::init<>());
  py::class_<OHaraRudyEpi, std::shared_ptr<OHaraRudyEpi>, OHaraRudy>(
      m_Cells, "OHaraRudyEpi",
              R"pbdoc(
Human Ventricular Epicardium (O'Hara-Rudy 2011)

| O’hara, Thomas, et al. “Simulation of the Undiseased Human Cardiac Ventricular
|    Action Potential: Model Formulation and Experimental Validation.” PLoS Comput
|    Biol, vol. 7, no. 5, American Heart Association, 2011, pp. 1002061–302,
|    doi:10.1371/journal.pcbi.1002061.)pbdoc")
      .def(py::init<>());
  py::class_<OHaraRudyM, std::shared_ptr<OHaraRudyM>, OHaraRudy>(m_Cells,
                                                                 "OHaraRudyM",
                R"pbdoc(
Human Ventricular Mid Myocardial (O'Hara-Rudy 2011)

| O’hara, Thomas, et al. “Simulation of the Undiseased Human Cardiac Ventricular
|    Action Potential: Model Formulation and Experimental Validation.” PLoS Comput
|    Biol, vol. 7, no. 5, American Heart Association, 2011, pp. 1002061–302,
|    doi:10.1371/journal.pcbi.1002061.)pbdoc")
      .def(py::init<>());
  py::class_<Courtemanche98, std::shared_ptr<Courtemanche98>, Cell>(
      m_Cells, "Courtemanche98", R"pbdoc(
Human Atrial (Courtemanche 1998)

| Courtemanche, Marc, et al. “Ionic Mechanisms Underlying Human Atrial Action
|   Potential Properties: Insights from a Mathematical Model.” The American Journal
|   of Physiology, vol. 275, no. 1 Pt 2, July 1998, pp. H301-21,
|   http://www.ncbi.nlm.nih.gov/pubmed/9688927.
)pbdoc")
      .def(py::init<>());
  py::class_<Br04, std::shared_ptr<Br04>, Cell>(m_Cells, "Br04", R"pbdoc(
Mouse Ventricular (Bondarenko 2004)

| Bondarenko, V. E. “Computer Model of Action Potential of Mouse Ventricular
|   Myocytes.” AJP: Heart and Circulatory Physiology, vol. 287, no. 3, 2004, pp.
|   H1378–403, doi:10.1152/ajpheart.00185.2003.
 )pbdoc")
      .def(py::init<>());
  py::class_<GpbAtrial, std::shared_ptr<GpbAtrial>, Cell>(m_Cells, "GpbAtrial", R"pbdoc(
Human Atrial (Grandi 2011)

| Grandi, E., et al. “Human Atrial Action Potential and Ca2+ Model: Sinus Rhythm
|   and Chronic Atrial Fibrillation.” Circulation Research, vol. 109, no. 9, 2011,
|   pp. 1055–66, doi:10.1161/CIRCRESAHA.111.253955.)pbdoc")
      .def(py::init<>());
  py::class_<GpbVent, std::shared_ptr<GpbVent>, Cell>(m_Cells, "GpbVent", R"pbdoc(
Human Ventricular (Grandi 10)

| Grandi, Eleonora, et al. A Novel Computational Model of the Human Ventricular
|   Action Potential and Ca Transient. 2009, doi:10.1016/j.yjmcc.2009.09.019.                                                                         )pbdoc")
      .def(py::init<>());
  py::class_<HRD09Control, std::shared_ptr<HRD09Control>, Cell>(m_Cells,
                                                                "HRD09Control", R"pbdoc(
Canine Ventricular (Hund-Rudy 2009)

| Hund, T. J., Decker, K. F., Kanter, E., Mohler, P. J., Boyden, P. A., Schuessler,
|   R. B., … Rudy, Y. (2008). Role of activated CaMKII in abnormal calcium homeostasis
|   and INa remodeling after myocardial infarction: Insights from mathematical modeling.
|   Journal of Molecular and Cellular Cardiology, 45(3), 420–428.
|   https://doi.org/10.1016/j.yjmcc.2008.06.007)pbdoc")
      .def(py::init<>());
  py::class_<HRD09BorderZone, std::shared_ptr<HRD09BorderZone>, HRD09Control>(
      m_Cells, "HRD09BorderZone",R"pbdoc(
Canine Ventricular Border Zone (Hund-Rudy 2009)

| Hund, T. J., Decker, K. F., Kanter, E., Mohler, P. J., Boyden, P. A., Schuessler,
|   R. B., … Rudy, Y. (2008). Role of activated CaMKII in abnormal calcium homeostasis
|   and INa remodeling after myocardial infarction: Insights from mathematical modeling.
|   Journal of Molecular and Cellular Cardiology, 45(3), 420–428.
|   https://doi.org/10.1016/j.yjmcc.2008.06.007)pbdoc")
      .def(py::init<>());
  py::class_<InexcitableCell, std::shared_ptr<InexcitableCell>, Cell>(
      m_Cells, "InexcitableCell", R"pbdoc(Inexcitable Cell
              ..Note: No Publication)pbdoc")
      .def(py::init<>());
  py::class_<CoupledInexcitableCell, std::shared_ptr<CoupledInexcitableCell>,
             Cell>(m_Cells, "CoupledInexcitableCell", R"pbdoc(Coupled Inexcitable Cell
                            ..Note: No Publication)pbdoc")
      .def(py::init<>());
  py::class_<Ksan, std::shared_ptr<Ksan>, Cell>(m_Cells, "Ksan", R"pbdoc(
Mouse Sinus Node (Kharche 2011)

| Kharche, Sanjay, et al. “A Mathematical Model of Action Potentials of Mouse
|   Sinoatrial Node Cells with Molecular Bases.” American Journal of
|   Physiology-Heart and Circulatory Physiology, vol. 301, no. 3, Sept. 2011, pp.
|   H945–63, doi:10.1152/ajpheart.00143.2010.)pbdoc")
      .def(py::init<>());
  py::class_<TNNP04Control, std::shared_ptr<TNNP04Control>, Cell>(
      m_Cells, "TNNP04Control", R"pbdoc(
Human Ventricular (Ten Tusscher 2004)

| Ten Tusscher, KHWJ H. W. J., et al. “A Model for Human Ventricular Tissue.”
|   American Journal of Physiology. Heart and Circulatory Physiology, vol. 286, no.
|   4, 2004, pp. H1573-89, doi:10.1152/ajpheart.00794.2003.)pbdoc")
      .def(py::init<>());
  py::class_<FR, std::shared_ptr<FR>, Cell>(m_Cells, "FaberRudy", R"pbdoc(
Mammalian Ventricular (Faber-Rudy 2000)

| Faber, Gregory M., and Yoram Rudy. “Action Potential and Contractility Changes
|   [Na+](i) Overloaded Cardiac Myocytes: A Simulation Study.” Biophysical Journal,
|   vol. 78, no. 5, Elsevier, 2000, pp. 2392–404,
|   doi:10.1016/S0006-3495(00)76783-X.)pbdoc")
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
      m_Cells, "GpbAtrialOnal17", R"pbdoc(
Human Atrial (Onal 2017)

| Onal, Birce, et al. “Ca 2+ /Calmodulin Kinase II-Dependent Regulation of Atrial
|   Myocyte Late Na+ Current, Ca 2+ Cycling and Excitability: A Mathematical
|   Modeling Study.” American Journal of Physiology - Heart and Circulatory
|   Physiology, 2017, p. ajpheart.00185.2017, doi:10.1152/ajpheart.00185.2017.)pbdoc")
      .def(py::init<>())
      .def("updateIna", &GpbAtrialOnal17::updateIna);

  py::class_<GridCell, std::shared_ptr<GridCell>, Cell>(m_Cells, "GridCell", R"pbdoc(
Grid Cell

| Henriquez, C. S., & Plonsey, R. (1987). Effect of resistive discontinuities
|   on waveshape and velocity in a single cardiac fibre. Medical & Biological
|   Engineering & Computing, 25(4), 428–438. https://doi.org/10.1007/BF02443364)pbdoc")
      .def(py::init<>())
// windows compiler static issues
#if !defined(_WIN32) && !defined(_WIN64)
      .def_property_readonly("grid", &GridCell::getGrid)
#endif
      ;
}
