# roboflex.audio_sdl

Support for reading audio data using Simple Directmedia Layer.

## System dependencies

Requires SDL to be installed. More than likely, it already is installed in your distro. But if not:

    apt-get install libsdl2-dev

    # or maybe on mac:

    brew install sdl2

## pip install

    pip install roboflex.audio_sdl

## Import

    import roboflex.audio_sdl as ras

## Nodes

There is only one: **AudioSensor**

    # all parameters optional: below are the defaults
    audio_sensor = ras.AudioSensor(
        capture_id = -1,
        channels = 1,
        sampling_rate = 48000,
        capture_samples = 512,
        format = ras.BitDepth.F32,
        name = "AudioSensor",
        data_key = "data",
        debug = False,
    )

    # must be started (like all sensors)!
    audio_sensor.start()

    # you can get the realized channels and sampling rate (might be different from what you requested):
    audio_sensor.sample_rate
    audio_sensor.channels

    # you can print the available detected devices, through a static method
    ras.AudioSensor.show_devices()

This sensor publishes a TensorMessage, with the audio data encoded into a tensor of shape (C, S), where C is num channels, and S is capture_samples, under the key <data_key>.


## Other

Available BitDepths:

    # wrapped from SDL_AudioFormat: https://wiki.libsdl.org/SDL2/SDL_AudioFormat

    enum ras.BitDepth:
        S8,
        U8,
        S16LSB,
        S16MSB,
        S16SYS,
        S16,
        U16LSB,
        U16MSB,
        U16SYS,
        U16,
        S32LSB,
        S32MSB,
        S32SYS,
        S32,
        F32LSB,
        F32MSB,
        F32SYS,
        F32