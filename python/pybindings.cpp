#include <string>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include "roboflex_core/core.h"
#include "roboflex_audio_sdl/audio_sdl.h"

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::core;
using namespace roboflex::audio_sdl;

PYBIND11_MODULE(roboflex_audio_sdl_ext, m) {
    m.doc() = "roboflex_audio_sdl_ext";

    py::enum_<AudioSensor::BitDepth>(m, "BitDepth")
        .value("S8", AudioSensor::BitDepth::S8)
        .value("U8", AudioSensor::BitDepth::U8)
        .value("S16LSB", AudioSensor::BitDepth::S16LSB)
        .value("S16MSB", AudioSensor::BitDepth::S16MSB)
        .value("S16SYS", AudioSensor::BitDepth::S16SYS)
        .value("S16", AudioSensor::BitDepth::S16)
        .value("U16LSB", AudioSensor::BitDepth::U16LSB)
        .value("U16MSB", AudioSensor::BitDepth::U16MSB)
        .value("U16SYS", AudioSensor::BitDepth::U16SYS)
        .value("U16", AudioSensor::BitDepth::U16)
        .value("S32LSB", AudioSensor::BitDepth::S32LSB)
        .value("S32MSB", AudioSensor::BitDepth::S32MSB)
        .value("S32SYS", AudioSensor::BitDepth::S32SYS)
        .value("S32", AudioSensor::BitDepth::S32)
        .value("F32LSB", AudioSensor::BitDepth::F32LSB)
        .value("F32MSB", AudioSensor::BitDepth::F32MSB)
        .value("F32SYS", AudioSensor::BitDepth::F32SYS)
        .value("F32", AudioSensor::BitDepth::F32)
    ;

    py::class_<AudioSensor, RunnableNode, std::shared_ptr<AudioSensor>>(m, "AudioSensor")
        .def(py::init<int, unsigned int, unsigned int, unsigned int, AudioSensor::BitDepth, const string&, const string&, bool>(),
            "Create an AudioSensor",
            py::arg("capture_id") = -1,
            py::arg("channels") = 1,
            py::arg("sampling_rate") = 44100,
            py::arg("capture_samples") = 512,
            py::arg("format") = AudioSensor::BitDepth::F32,
            py::arg("name") = "AudioSensor",
            py::arg("data_key") = "data",
            py::arg("debug") = false)
    ;
}

