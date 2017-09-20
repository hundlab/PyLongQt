#include "structure/node.h"
#include "structure/fiber.h"
#include "structure/grid.h"
#include "pylongqt.h"

void init_structures(py::module &m) {
    py::module m_Structures = m.def_submodule("Structures", "Structures used by GridProtocol for 1 & 2D simulations.");

    py::class_<Node, std::shared_ptr<Node>>(m_Structures, "Node", "Finest unit in a multi-dimensional simulation, contains a cell")
        .def(py::init<>())
        .def("setCondConst", &Node::setCondConst, "set the conductivity values", py::arg("X"),py::arg("dx"),py::arg("s"),py::arg("perc")=true,py::arg("val")=1)
        .def_readonly("cell", &Node::cell)
    //            .def_readwrite("rd",&Node::rd)
        .def_readwrite("condConst", &Node::condConst)
        .def_readwrite("np",&Node::np,"number of cells represented by the node")
        .def_readwrite("type",&Node::nodeType)
        .def("__repr__",[](Node& n){return "<Node "+string(py::repr(py::cast(n.cell)))+">";});
    py::class_<Fiber>(m_Structures, "Fiber", "A 1D line of nodes")
        .def(py::init<int>())
        .def("updateVm",&Fiber::updateVm,"Update voltages for all nodes")
    //            .def_readwrite("nodes",&Fiber::nodes)
        .def_readwrite("B",&Fiber::B)
        .def("__len__",&Fiber::size)
        .def("__getitem__",&Fiber::operator[])
        .def("__iter__", [](const Fiber& f)
            { return py::make_iterator(f.begin(), f.end()); },
            py::keep_alive<0, 1>())
        .def("__repr__", [](Fiber& f){
            string repr = "Fiber([";
            bool first = true;
            for(auto& node: f.nodes) {
                if(!first) repr+=", ";
                repr+=string(py::repr(py::cast(node)));
                first=false;
            }
            repr += "])";
            return repr;
        });
    py::class_<CellInfo>(m_Structures, "CellInfo", "Contains all the info needed to insert a cell into a Grid")
        .def(py::init<int,int,double,double,int,shared_ptr<Cell>,array<double,4>,bool>(),py::arg("X")=-1,py::arg("Y")=-1,py::arg("dx")=0.01,py::arg("dy")=0.01,py::arg("np")=1,py::arg("cell")=nullptr,py::arg("c")=array<double,4>({NAN,NAN,NAN,NAN}),py::arg("c_perc")=false)
        .def_readwrite("row",&CellInfo::row)
        .def_readwrite("col",&CellInfo::col)
        .def_readwrite("dx",&CellInfo::dx)
        .def_readwrite("dy",&CellInfo::dy)
        .def_readwrite("np",&CellInfo::np)
        .def_readwrite("c_perc",&CellInfo::c_perc)
        .def_readwrite("cell",&CellInfo::cell)
        .def_readwrite("c",&CellInfo::c)
        .def("__repr__", [](const CellInfo& r)
            {return "<CellInfo row="+to_string(r.row)+", col="+to_string(r.col)+
            (r.cell? ", cell='"+string(r.cell->type())+"'>": "");});

    py::class_<Grid>(m_Structures, "Grid")
        .def(py::init<>())
        .def("addRow",&Grid::addRow,py::arg("pos"))
        .def("addRows",&Grid::addRows,py::arg("num"),py::arg("position")=0)
        .def("addColumn",&Grid::addColumn,py::arg("pos"))
        .def("addColumns",&Grid::addColumns,py::arg("num"),py::arg("position")=0)
        .def("removeRow",&Grid::removeRow,py::arg("pos"))
        .def("removeRows",&Grid::removeRows,py::arg("num"),py::arg("pos"))
        .def("removeColumn",&Grid::removeColumn,py::arg("pos"))
        .def("removeColumns",&Grid::removeColumns,py::arg("num"),py::arg("pos"))
        .def("setCells",(void(Grid::*)(list<CellInfo>&))&Grid::setCellTypes,"set cells in grid",py::arg("cells"))
        .def("setCell",(void(Grid::*)(const CellInfo&))&Grid::setCellTypes,py::arg("singleCell"))
        .def_property_readonly("shape",[](Grid& g){return make_tuple(g.rowCount(),g.columnCount());})
        .def("rowCount",&Grid::rowCount)
        .def("columnCount",&Grid::columnCount)
        .def("findNode",&Grid::findNode,"find a nodes position")
        .def("reset",&Grid::reset)
        .def("updateB",&Grid::updateB,"use node conductivity values to update fiber conductivity list")
        .def("__len__",&Grid::columnCount)
        .def("__iter__", [](const Grid& g)
            { return py::make_iterator(g.begin(), g.end()); },
            py::keep_alive<0, 1>())
        .def("__getitem__",(shared_ptr<Node>(Grid::*)(const int, const int))&Grid::operator())
        .def("__getitem__",[](Grid& g,list<pair<int,int>> l){
            list<shared_ptr<Node>> resList;
            for(auto& item: l) {
                resList.push_back(g(item));
            }
            return resList;
        })
        .def("__repr__",[](Grid& g){
             string repr = "Grid([";
             bool first = true;
             for(Fiber& f: g.rows) {
                if(!first) repr+=", ";
                repr+=string(py::repr(py::cast(f)));
                first = false;
             }
             repr+="])";
             return repr;
        });
}
