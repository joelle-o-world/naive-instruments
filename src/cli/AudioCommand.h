#include "../Waveforms.h"
#include "../core.h"
#include "../file-io/piping.h"
#include "../file-io/record.h"
#include "../instruments/PitchConverter.h"
#include "../parsing/Parse.h"
#include "../playback/BufferedPlayback.h"
#include "./SignalString.h"
#include "Command.h"

class AudioCommand : public Command {
public:
  using Command::Command;

protected:
  void describeOutputOptions() {
    options.add_options()("normalize", "Normalize the audio output")(
        "duration", po::value<std::string>(),
        "Crop the audio output to a certain duration")(
        "output-file,o", po::value<std::string>(),
        "Path to wav file for recording audio\n");
  }

  /**
   * Three ways to get a primary input into a file.
   *   1) --input <file>
   *   2) pipe a wav file to stdin
   *   3) use the positional argument
   */
  void descripeInputOptions() {
    options.add_options()("input", po::value<std::string>(),
                          "The main input signal");
    if (!stdinIsAPipe())
      positionalOptions.add("input", 1);
  }

  void addWaveformOptions() {
    options.add_options()(
        "waveform", po::value<std::string>()->default_value("sine"),
        "Set the oscillator's waveform (sine|saw|square|triangle)");
  }

  MonoBuffer *inputWaveform() {
    return &Waveforms::byName(args["waveform"].as<std::string>());
  }

  void addPitchOptions() {
    options.add_options()(
        "pitch", po::value<std::string>()->default_value("50"),
        "Frequency of the oscillator expressed as a midi pitch number.");
  }

  shared_ptr<Signal<double>> inputPitch() {
    return SignalString::parse(args["pitch"].as<std::string>());
  }

  shared_ptr<Signal<double>> inputFrequency() {
    shared_ptr<Signal<double>> pitch = inputPitch();
    shared_ptr<PitchConverter> converter = make_shared<PitchConverter>();
    converter->pitch << pitch;
    return converter;
  }

  void addVibratoOptions() {
    options.add_options()("vibrato-frequency,vf",
                          po::value<std::string>()->default_value("5"),
                          "Vibrato frequency")(
        "vibrato-amount,va", po::value<std::string>()->default_value("0"),
        "Amount of vibrato (in semitones)");
  }

  /**
   * Get the primary audio input as a buffer.
   */
  MonoBuffer *inputAsBuffer() {
    if (args.count("input")) {
      const std::string filename = args["input"].as<std::string>();
      return WavReader::readMonoFile(filename);
    } else if (stdinIsAPipe()) {
      return WavReader::readStream(std::cin);
    } else {
      cerr << "No input!\n";
      throw 1;
    }
  }

  const Hopefully<std::string> outputFile() {
    if (args.count("output-file"))
      return args["output-file"].as<std::string>();
    else
      return Disappointment;
  }

  float outputDuration() {
    std::string durationStr = args["duration"].as<std::string>();
    return *Parse::interval(durationStr);
  }

  // TODO: Cleary you are getting confused Joelle
  float inputDuration() {
    if (args.count("duration"))
      return outputDuration();
    else
      return 1;
  }

protected:
  // TODO: Use a shared ptr?
  void output(Signal<double> &signal) {
    auto &path = outputFile();
    if (stdoutIsAPipe()) {
      std::cerr << "Piping to stdout\n";
      record(std::cout, signal, outputDuration());
    } else if (!path.success())
      BufferedPlayback::play(signal);
    else {
      record(*path, signal, outputDuration());
      std::cout << *path << "\n";
    }
  }
};
