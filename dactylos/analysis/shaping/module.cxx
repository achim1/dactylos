#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>
#include <pybind11/numpy.h>

#include "trapezoidal_shaper.h"

namespace py = pybind11;
PYBIND11_MODULE(_trapezoidal_shaper, m) {
    m.doc() = "Fast C++ version of trapezoidal shaping routine. Similar to trapezoidal_shaper.py";

    py::class_<TrapezoidalFilter>(m, "TrapezoidalFilter")
        .def(py::init<int, int, int>())//,
            // FIXME: the proper constructor with keyword support 
            // py::arg("ptime"), py::kwarg("flat")=1000., py::kwarg("recordlength")=50000
            //)
        .def("shape_it", &TrapezoidalFilter::shape_it)
        // we need __getstate__ and __setstate__ so that we are capable of pickling our class
        // - this is important for the use with python multiprocessing module, 
        // since this requires pickleable objects.
        .def("__getstate__", [](const TrapezoidalFilter &t) {
            return py::make_tuple(t.ptime, t.flat, t.recordlength);
        })
        .def("__setstate__", [](TrapezoidalFilter &trap, py::tuple t) {
            if (t.size() != 3)
                throw std::runtime_error("Invalid state!");
            new (&trap) TrapezoidalFilter(t[0].cast<int>(),t[1].cast<int>(),t[2].cast<int>());
        });

}


