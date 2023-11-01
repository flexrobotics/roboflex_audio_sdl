#include <iostream>
#include <sstream>
#include <xtensor/xtensor.hpp>
#include <xtensor/xadapt.hpp>
#include "roboflex_audio_sdl/audio_sdl.h"
#include "roboflex_core/serialization/flex_eigen.h"
#include "roboflex_core/serialization/flex_utils.h"
#include "roboflex_core/util/utils.h"
#include "roboflex_core/core_messages/core_messages.h"

namespace roboflex {
namespace audio_sdl {


// --- utilities ---

std::string shape_to_string(const std::vector<size_t>& shape)
{
    std::stringstream ss;
    ss << "(";
    for (size_t i = 0; i < shape.size(); i++) {
        ss << shape[i];
        if (i < shape.size()-1) {
            ss << ", ";
        }
    }
    ss << ")";
    return ss.str();
}

std::string audio_format_to_string(const SDL_AudioFormat& f)
{
    switch (f) {
        case AUDIO_S8:     return "AUDIO_S8";
        case AUDIO_U8:     return "AUDIO_U8";
        case AUDIO_S16LSB: return "AUDIO_S16LSB";
        case AUDIO_S16MSB: return "AUDIO_S16MSB";
        case AUDIO_U16LSB: return "AUDIO_U16LSB";
        case AUDIO_U16MSB: return "AUDIO_U16MSB";
        case AUDIO_S32LSB: return "AUDIO_S32LSB";
        case AUDIO_S32MSB: return "AUDIO_S32MSB";
        case AUDIO_F32LSB: return "AUDIO_F32LSB";
        case AUDIO_F32MSB: return "AUDIO_F32MSB";
    }
    return "UNKNOWN";
}

std::string sdl_to_string(const SDL_AudioFormat& f)
{
    std::stringstream sst;
    sst << "<SDL_AudioFormat " << audio_format_to_string(f)
        << " signed:" << !(bool)SDL_AUDIO_ISUNSIGNED(f) 
        << " bigendian:" << (bool)SDL_AUDIO_ISBIGENDIAN(f)
        << " float:" << (bool)SDL_AUDIO_ISFLOAT(f)
        << " bitwidth:" << SDL_AUDIO_BITSIZE(f)
        << ">";
    return sst.str();
}

std::string sdl_to_string(const SDL_AudioSpec& spec) 
{    
    std::stringstream sst;
    sst << " freq: " << spec.freq << std::endl
        << " format: " << spec.format << " " << sdl_to_string(spec.format) << std::endl
        << " channels: " << (int)spec.channels << std::endl
        << " silence: " << (int)spec.silence << std::endl
        << " samples: " << (int)spec.samples << std::endl
        << " size: " << spec.size << std::endl;
    return sst.str();
}

template <typename T>
std::shared_ptr<core::TensorMessage<T, 2>> audio_message(uint8_t* stream, int len, unsigned int channels, const string& data_key)
{
    const T * data_typed = (const T*)stream;
    const size_t typed_size = len / sizeof(T);
    const std::vector<size_t> vshape = {channels, typed_size / channels};

    // adapter just adapts the given data, but assignment
    // copies it into an xtensor. OR DOES IT?
    // Just what is tensor?

    // ALSO: I'm sure there's bound to be an error with strides...
    // SDL encodes stereo as LRLRLRLR...

    auto tensor = xt::adapt(
        data_typed,
        typed_size,
        xt::no_ownership(),
        vshape);

    return std::make_shared<core::TensorMessage<T, 2>>(tensor, "TensorAudioData", data_key);
}

core::MessagePtr get_audio_message(uint8_t* stream, int len, unsigned int channels, SDL_AudioFormat format, const string& data_key)
{
    switch (format) {
        case AUDIO_S8:     return audio_message<int8_t>(stream, len, channels, data_key);
        case AUDIO_U8:     return audio_message<uint8_t>(stream, len, channels, data_key);
        case AUDIO_S16LSB: return audio_message<int16_t>(stream, len, channels, data_key);
        case AUDIO_S16MSB: return audio_message<int16_t>(stream, len, channels, data_key);
        case AUDIO_U16LSB: return audio_message<uint16_t>(stream, len, channels, data_key);
        case AUDIO_U16MSB: return audio_message<uint16_t>(stream, len, channels, data_key);
        case AUDIO_S32LSB: return audio_message<int32_t>(stream, len, channels, data_key);
        case AUDIO_S32MSB: return audio_message<int32_t>(stream, len, channels, data_key);
        case AUDIO_F32LSB: return audio_message<float>(stream, len, channels, data_key);
        case AUDIO_F32MSB: return audio_message<float>(stream, len, channels, data_key);
    }
    return nullptr;
}


// --- AudioSensor ---

bool AudioSensor::initialized = false;

AudioSensor::AudioSensor(
    int capture_id,
    unsigned int channels,
    unsigned int sampling_rate,
    unsigned int capture_samples,
    AudioSensor::BitDepth format,
    const string& name,
    const string& data_key,
    bool debug):
        core::RunnableNode(name),
        capture_id(capture_id),
        data_key(data_key),
        debug(debug)
{
    AudioSensor::initialize();

    // what does it do?
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        string err = string("Couldn't initialize SDL: ") + SDL_GetError();
        throw std::runtime_error(err);
    }

    // what does it do?
    SDL_SetHintWithPriority(SDL_HINT_AUDIO_RESAMPLING_MODE, "medium", SDL_HINT_OVERRIDE);

    SDL_AudioSpec capture_spec_requested;
    SDL_AudioSpec capture_spec_received;

    SDL_zero(capture_spec_requested);
    SDL_zero(capture_spec_received);

    capture_spec_requested.freq = sampling_rate;
    capture_spec_requested.format = (SDL_AudioFormat)format;
    capture_spec_requested.channels = channels;
    capture_spec_requested.samples = capture_samples;
    capture_spec_requested.userdata = this;
    capture_spec_requested.callback = [](void * userdata, uint8_t * stream, int len) {
        AudioSensor * thiz = (AudioSensor *)userdata;
        thiz->audio_callback(stream, len);
    };

    if (capture_id >= 0) {
        device_id = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(capture_id, SDL_TRUE), SDL_TRUE, &capture_spec_requested, &capture_spec_received, 0);
    } else {
        device_id = SDL_OpenAudioDevice(nullptr, SDL_TRUE, &capture_spec_requested, &capture_spec_received, 0);
    }

    if (!device_id) {
        device_id = 0;
        throw std::runtime_error(string("couldn't open an audio device for capture, err=") + SDL_GetError());
    } else {
        if (debug) {
            string capstr = capture_id >= 0 ? (string(", '") + SDL_GetAudioDeviceName(capture_id, SDL_TRUE) + "'") : ", <any>";
            std::cerr << "AudioSensor (SDL) requested capture device: " << capture_id << capstr << " with spec:" << std::endl;
            std::cerr << sdl_to_string(capture_spec_requested);
            std::cerr << "AudioSensor (SDL) received device id " << device_id << " with spec:" << std::endl;
            std::cerr << sdl_to_string(capture_spec_received);
        }
    }

    this->sample_rate = capture_spec_received.freq;
    this->channels = capture_spec_received.channels;
    this->format = capture_spec_received.format;
}
    
AudioSensor::~AudioSensor()
{
    if (device_id) {
        SDL_CloseAudioDevice(device_id);
    }
}

void AudioSensor::initialize()
{
    if (!initialized) {
        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            string err = string("Couldn't initialize SDL: ") + SDL_GetError();
            throw std::runtime_error(err);
        }
    }
}

void AudioSensor::show_devices()
{
    AudioSensor::initialize();

    int nDevices = SDL_GetNumAudioDevices(SDL_TRUE);
    std::cerr << "AudioSensor (SDL) found " << nDevices << " capture devices:" << std::endl;
    for (int i = 0; i < nDevices; i++) {
        std::cerr << "  Capture device " << i << ": '" << SDL_GetAudioDeviceName(i, SDL_TRUE) << "'" << std::endl;
    }
}

void AudioSensor::audio_callback(uint8_t* stream, int len)
{
    auto msg = get_audio_message(stream, len, this->channels, this->format, this->data_key);
    this->signal(msg);
}

void AudioSensor::start()
{
    SDL_PauseAudioDevice(device_id, 0);
}

void AudioSensor::stop()
{
    SDL_PauseAudioDevice(device_id, 1);
}

} // namespace audio_sdl
} // namespace roboflex
