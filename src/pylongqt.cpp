#include "pylongqt.h"
#include "cellutils.h"
#include <string>
#include "inexcitablecell.h"
#include "measuremanager.h"
#include "measure.h"
#include "protocol.h"
#include "logger.h"
#include <iostream>

using namespace LongQt;
using std::string;
using std::make_shared;
using std::set;
using std::map;

auto measMap = [] (string name) {
        auto c = make_shared<InexcitableCell>();
        auto meas = new MeasureManager(c);
        map<string,set<string>> measMap;
        for(auto var: meas->varsMeas) {
            measMap[var.first] = meas->varMeasCreator.at(var.second)({})->variables();
        }
        set<string> defaultMeas = Measure().variables();
        delete meas;
        return measMap.count(name) > 0? measMap[name]: defaultMeas;
    };

PYBIND11_MODULE(PyLongQt, m) {
    auto logger = Logger::getInstance();
    logger->exceptionEnabled = true;

    m.doc() = "python bindings for LongQt's cell models and protocols";
    m.attr("cellMap") = py::cast(CellUtils::cellMap, py::return_value_policy::take_ownership);
    m.attr("protoMap") = py::cast(CellUtils::protoMap, py::return_value_policy::take_ownership);
    m.def("measMap",measMap, "Get the available measure variables for a given cell variable", py::arg("name"));
    py::enum_<CellUtils::Side>(m,"Side","Numbering of sides in 2D grids")
        .value("top", CellUtils::top)
        .value("bottom", CellUtils::bottom)
        .value("right", CellUtils::right)
        .value("left", CellUtils::left)
        .export_values();
    m.def("verbose", [] (bool on) {
        on ?
            Logger::getInstance()->STDOut(&std::cout):
            Logger::getInstance()->delSTDOut();});
    init_cells(m);
    init_protocols(m);
    init_structures(m);
    init_misc(m);
}
