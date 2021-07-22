#ifndef PYLONGQT
#define PYLONGQT
#include <cmath>
#undef slots
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#define slots Q_SLOTS
#include <memory>

namespace py = pybind11;

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

void init_cells(py::module &m);
void init_protocols(py::module &m);
void init_structures(py::module &m);
void init_measures(py::module &m);
void init_pvars(py::module &m);
#endif
