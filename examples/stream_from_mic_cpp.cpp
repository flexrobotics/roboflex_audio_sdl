#include <roboflex_audio_sdl/audio_sdl.h>
#include <unistd.h>
#include <roboflex_transport_zmq/zmq_nodes.h>
#include <roboflex_core/core_nodes/message_printer.h>

int main() {

    roboflex::audio_sdl::AudioSensor::show_devices();

    auto audio_sensor = roboflex::audio_sdl::AudioSensor(
        1,              // device index
        2,              // channels
        44100,          // frequency
        512,            // capture_samples
        roboflex::audio_sdl::AudioSensor::BitDepth::F32,      // format
        "AudioSensor",  // name
        "data",         // data_key
        true            // debug
    );

    auto zmq_context = roboflex::transportzmq::MakeZMQContext();
    auto zmq_pub = roboflex::transportzmq::ZMQPublisher(
        zmq_context, 
        "tcp://*:5555",     // bind address to subscribe to
        "ZMQPublisher",     // node name
        2                   // queue size
    );

    // auto msg_printer = roboflex::nodes::MessagePrinter();

    audio_sensor > zmq_pub;

    audio_sensor.start();

    sleep(300);

    audio_sensor.stop();

    return 0;
}