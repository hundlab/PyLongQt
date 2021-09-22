#include "pylongqt.h"
#include "structure/fiber.h"
#include "structure/grid.h"
#include "structure/node.h"
using namespace LongQt;
using namespace std;

void init_structures(py::module& m) {
  py::module m_Structures = m.def_submodule(
      "Structures", "Structures used by GridProtocol to layout 1 & 2D simulations.");

  py::class_<Node, std::shared_ptr<Node>>(
      m_Structures, "Node",
      "The wrapper for a single cell in a multi-dimensional simulation")
      //      .def(py::init<>())
      .def("setCondConst", &Node::setCondConst,
           R"pbdoc(Set the conductivity (what are the units) of a side to a value
           :side: The side to set
           :perc: If the value is a precentage or raw
           :value: The value)pbdoc", py::arg("side"), py::arg("perc") = true, py::arg("value") = 1)
      .def("getCondConst", &Node::getCondConst,
           R"pbdoc(Get the conductivity of a side
           :side: The side to get)pbdoc", py::arg("side"))
      .def("resetCondConst", &Node::resetCondConst,
           R"pbdoc(Reset the conductivity of a side
           :side: The side to reset (if left as the default option all sides will be reset)
           )pbdoc", py::arg("side") = -1)
      .def("cellOptions", &Node::cellOptions)
      .def_property(
          "cell",
          static_cast<shared_ptr<Cell> (Node::*)(void) const>(&Node::cell),
          (void (Node::*)(shared_ptr<Cell>)) &Node::cell,
          R"pbdoc(Get or set the cell contained in the node)pbdoc")
      .def("setCellByName",
           (bool (Node::*)(const std::string&)) &Node::cell,
           R"pubdoc(Set the cell using its name rather than a cell model object
           :name: The name of the cell model)pubdoc",
           py::arg("cellName"))
      //            .def_readwrite("rd",&Node::rd)
      //        .def_readwrite("condConst", &Node::condConst)
      .def("__repr__", [](Node& n) {
        return "<Node " + string(py::repr(py::cast(n.cell()))) + ">";
      });
  py::class_<Fiber>(m_Structures, "Fiber",
                    R"pbdoc(A 1D row/column of nodes in a :py:class:`Grid`)pbdoc")
      .def(py::init<int, LongQt::CellUtils::Side>())
      .def("updateVm", &Fiber::updateVm, R"pbdoc(Update voltages for all nodes. Calls :py:meth:`Cell.updateVm`)pbdoc")
      //            .def_readwrite("nodes",&Fiber::nodes)
      //        .def_readwrite("B",&Fiber::B)
      .def("__len__", &Fiber::size, "The number of nodes in the fiber")
      .def("__getitem__", &Fiber::operator[], "Get a node by index", py::arg("index"))
      .def("__iter__",
           [](Fiber& f) { return py::make_iterator(f.begin(), f.end()); },
            "Iterate over nodes",
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

  py::class_<Grid>(m_Structures, "Grid",
                   "A 2D plane of nodes, which provides the layout for 1 and 2 dimentional simulations")
      .def(py::init<>())
      .def("addRow", &Grid::addRow, "Add a single row of cells")
      .def("addRows", &Grid::addRows,
           R"pbdoc(Add multiple rows of cells
           :num: the number of rows to add)pbdoc",
           py::arg("num"))
      .def("addColumn", &Grid::addColumn, "Add a single column of cells")
      .def("addColumns", &Grid::addColumns,
           R"pbdoc(Add multiple columns of cells
           :num: the number of rows to add)pbdoc",
           py::arg("num"))
      .def("removeRow", &Grid::removeRow,
           R"pbdoc(Remove one row of cells
           :index: the location of the row to remove)pbdoc",
           py::arg("index"))
      .def("removeRows", &Grid::removeRows,
           R"pbdoc(Remove multiple rows of cells
           :num: the number of rows including index to be removed
           :index: the location of the first row to remove)pbdoc",
           py::arg("num"), py::arg("index"))
      .def("removeColumn", &Grid::removeColumn,
           R"pbdoc(Remove one column of cells
           :index: the location of the column to remove)pbdoc",
           py::arg("index"))
      .def("removeColumns", &Grid::removeColumns,
           R"pbdoc(Remove multiple columns of cells
           :num: the number of columns including index to be removed
           :index: the location of the first column to remove)pbdoc",
           py::arg("num"), py::arg("index"))
      .def_property_readonly(
          "shape",
          [](Grid& g) { return make_tuple(g.rowCount(), g.columnCount()); },
        "The shape of the grid (rows, columns)")
      .def_property_readonly("size",
          [](Grid& g) { return g.rowCount()* g.columnCount(); })
      .def_readwrite("dx", &Grid::dx, "The length of each node in the grid")
      .def_readwrite("dy", &Grid::dy, "The width of each cell in the grid")
      .def_readwrite("np", &Grid::np, "WHAT IS THIS")
      .def("rowCount", &Grid::rowCount, "The number of rows")
      .def("columnCount", &Grid::columnCount, "The number of columns")
      .def("findNode", &Grid::findNode,
           R"pbdoc(Find a node's location in the grid
           :node: The node object to locate)pbdoc",
           py::arg("node"))
      .def("reset", &Grid::reset, "Reset the grid to original state")
      .def("__len__", &Grid::columnCount, "Number of columns")
      .def("__iter__",
           [](Grid& g) { return py::make_iterator(g.begin(), g.end()); },
            "Iterate over nodes",
           py::keep_alive<0, 1>())
      .def("__getitem__", (shared_ptr<Node>(Grid::*)(const pair<int, int>&)) &
                              Grid::operator(),
           "Retrieve node by its (row, column)")
      .def("__getitem__",
           [](Grid& g, list<pair<int, int>> l) {
             list<shared_ptr<Node>> resList;
             for (auto& item : l) {
               resList.push_back(g(item));
             }
             return resList;
           },
           "Retrieve nodes by thier [(row, column),...]")
      //      .def("__setitem__",
      //           [](Grid& g, const pair<int, int>& pos, std::shared_ptr<Cell>
      //           cell) {
      //             CellInfo info(pos.first, pos.second);
      //             info.cell = cell;
      //             g.setCellTypes(info);
      //           })
      .def("getRow", [](Grid& g, int i) { return &g.rows[i]; },
           R"pbdoc(Retrieve a row of nodes by thier index
           :index: The location of the row)pbdoc",
           py::arg("index"),
           py::return_value_policy::reference_internal)
      .def("getColumn", [](Grid& g, int i) { return &g.columns[i]; },
           R"pbdoc(Retrieve a column of nodes by thier index
           :index: The location of the column)pbdoc",
           py::arg("index"),
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
