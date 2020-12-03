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

#include <pybind11/iostream.h>

using namespace LongQt;
using std::make_shared;
using std::map;
using std::set;
using std::string;

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

PYBIND11_MODULE(PyLongQt, m) {
  auto logger = Logger::getInstance();
  logger->exceptionEnabled = true;

  m.doc() = "python bindings for LongQt's cell models and protocols";
  m.attr("cellMap") =
      py::cast(CellUtils::cellMap, py::return_value_policy::take_ownership);
  m.attr("protoMap") =
      py::cast(CellUtils::protoMap, py::return_value_policy::take_ownership);
//  m.def("measMap", measMap,
//        "Get the available measure variables for a given cell variable",
//        py::arg("name"));
  py::enum_<CellUtils::Side>(m, "Side", "Numbering of sides in 2D grids")
      .value("top", CellUtils::top)
      .value("bottom", CellUtils::bottom)
      .value("right", CellUtils::right)
      .value("left", CellUtils::left)
      .export_values();
  //this relies on a pybind11 detail
  m.def("verbose", &setVerbose);
  m.attr("version") = CellUtils::version;
  m.attr("max_print_lines") = 20;

  // py::register_exception<std::runtime_error>(m, "RuntimeException");
  init_cells(m);
  init_protocols(m);
  init_structures(m);
  init_misc(m);
  setVerbose(true);
}
