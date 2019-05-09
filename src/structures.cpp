#include "pylongqt.h"
#include "structure/fiber.h"
#include "structure/grid.h"
#include "structure/node.h"
using namespace LongQt;
using namespace std;

void init_structures(py::module& m) {
  py::module m_Structures = m.def_submodule(
      "Structures", "Structures used by GridProtocol for 1 & 2D simulations.");

  py::class_<Node, std::shared_ptr<Node>>(
      m_Structures, "Node",
      "Finest unit in a multi-dimensional simulation, contains a cell")
      //      .def(py::init<>())
      .def("setCondConst", &Node::setCondConst,
           "set the conductivity value of a side", py::arg("s"),
           py::arg("perc") = true, py::arg("val") = 1)
      .def("getCondConst", &Node::getCondConst,
           "get the conductivity value of a side", py::arg("s"))
      .def_readonly("cell", &Node::cell)
      //            .def_readwrite("rd",&Node::rd)
      //        .def_readwrite("condConst", &Node::condConst)
      .def("__repr__", [](Node& n) {
        return "<Node " + string(py::repr(py::cast(n.cell))) + ">";
      });
  py::class_<Fiber>(m_Structures, "Fiber", "A 1D line of nodes")
      .def(py::init<int, LongQt::CellUtils::Side>())
      .def("updateVm", &Fiber::updateVm, "Update voltages for all nodes")
      //            .def_readwrite("nodes",&Fiber::nodes)
      //        .def_readwrite("B",&Fiber::B)
      .def("__len__", &Fiber::size)
      .def("__getitem__", &Fiber::operator[])
      .def("__iter__",
           [](Fiber& f) { return py::make_iterator(f.begin(), f.end()); },
           py::keep_alive<0, 1>())
      .def("__repr__", [](Fiber& f) {
        string repr = "Fiber([";
        bool first = true;
        for (auto& node : f.nodes) {
          if (!first) repr += ", ";
          repr += string(py::repr(py::cast(node)));
          first = false;
        }
        repr += "])";
        return repr;
      });

  py::class_<Grid>(m_Structures, "Grid")
      .def(py::init<>())
      .def("addRow", &Grid::addRow)
      .def("addRows", &Grid::addRows, py::arg("num"))
      .def("addColumn", &Grid::addColumn)
      .def("addColumns", &Grid::addColumns, py::arg("num"))
      .def("removeRow", &Grid::removeRow, py::arg("pos"))
      .def("removeRows", &Grid::removeRows, py::arg("num"), py::arg("pos"))
      .def("removeColumn", &Grid::removeColumn, py::arg("pos"))
      .def("removeColumns", &Grid::removeColumns, py::arg("num"),
           py::arg("pos"))
      .def_property_readonly(
          "shape",
          [](Grid& g) { return make_tuple(g.rowCount(), g.columnCount()); })
      .def_readwrite("dx", &Grid::dx)
      .def_readwrite("dy", &Grid::dy)
      .def_readwrite("np", &Grid::np)
      .def("rowCount", &Grid::rowCount)
      .def("columnCount", &Grid::columnCount)
      .def("findNode", &Grid::findNode, "find a nodes position")
      .def("reset", &Grid::reset)
      .def("__len__", &Grid::columnCount)
      .def("__iter__",
           [](Grid& g) { return py::make_iterator(g.begin(), g.end()); },
           py::keep_alive<0, 1>())
      .def("__getitem__", (shared_ptr<Node>(Grid::*)(const pair<int, int>&)) &
                              Grid::operator())
      .def("__getitem__",
           [](Grid& g, list<pair<int, int>> l) {
             list<shared_ptr<Node>> resList;
             for (auto& item : l) {
               resList.push_back(g(item));
             }
             return resList;
           })
//      .def("__setitem__",
//           [](Grid& g, const pair<int, int>& pos, std::shared_ptr<Cell> cell) {
//             CellInfo info(pos.first, pos.second);
//             info.cell = cell;
//             g.setCellTypes(info);
//           })
      .def("getRow", [](Grid& g, int i) { return &g.rows[i]; },
           py::return_value_policy::reference_internal)
      .def("getColumn", [](Grid& g, int i) { return &g.columns[i]; },
           py::return_value_policy::reference_internal)
      .def("__repr__", [](Grid& g) {
        string repr = "Grid([";
        bool first = true;
        for (Fiber& f : g.rows) {
          if (!first) repr += ", ";
          repr += string(py::repr(py::cast(f)));
          first = false;
        }
        repr += "])";
        return repr;
      });
}
