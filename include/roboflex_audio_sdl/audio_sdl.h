#ifndef ROBOFLEX_AUDIO_SDL__H
#define ROBOFLEX_AUDIO_SDL__H

#include <Eigen/Dense>
#include <SDL2/SDL.h>
#include "roboflex_core/node.h"
#include "roboflex_core/serialization/flex_eigen.h"

namespace roboflex {
namespace audio_sdl {

using std::string, std::exception, std::ostream, std::vector;

constexpr char ModuleName[] = "audio_sdl";

/**
 * Reads from audio sensors and publishes core::nodes::TensorMessage.
 * Currently uses SDL2.
 */
class AudioSensor: public core::RunnableNode {
public:

    enum class BitDepth {
        S8 = AUDIO_S8,
        U8 = AUDIO_U8,
        S16LSB = AUDIO_S16LSB,
        S16MSB = AUDIO_S16MSB,
        S16SYS = AUDIO_S16SYS,
        S16 = AUDIO_S16,
        U16LSB = AUDIO_U16LSB,
        U16MSB = AUDIO_U16MSB,
        U16SYS = AUDIO_U16SYS,
        U16 = AUDIO_U16,
        S32LSB = AUDIO_S32LSB,
        S32MSB = AUDIO_S32MSB,
        S32SYS = AUDIO_S32SYS,
        S32 = AUDIO_S32,
        F32LSB = AUDIO_F32LSB,
        F32MSB = AUDIO_F32MSB,
        F32SYS = AUDIO_F32SYS,
        F32 = AUDIO_F32
    };

    AudioSensor(
        int capture_id = -1,
        unsigned int channels = 1,
        unsigned int sampling_rate = 44100,
        unsigned int capture_samples = 512,
        BitDepth format = BitDepth::F32,
        const string& name = "AudioSensor",
        const string& data_key = "data",
        bool debug = false);

    virtual ~AudioSensor();

    static void show_devices();

    void start() override;
    void stop() override;

    unsigned int get_sample_rate() const { return sample_rate; }

protected:

    static void initialize();

    void audio_callback(uint8_t* stream, int len);

    SDL_AudioDeviceID device_id = 0;

    int capture_id;
    unsigned int channels;
    unsigned int sample_rate;
    SDL_AudioFormat format;
    string data_key = "data";
    bool debug;

    static bool initialized;
};

} // namespace audio_sdl
} // namespace roboflex

#endif // ROBOFLEX_AUDIO_SDL__H
