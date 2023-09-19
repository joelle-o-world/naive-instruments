#pragma once

#include "../Waveforms.h"
#include "../core.h"
#include "../file-io/piping.h"
#include "../file-io/record.h"
#include "../instruments/PitchConverter.h"
#include "../parsing/Parse.h"
#include "../playback/BufferedPlayback.h"
#include "../playback/InteractivePlayback.h"
#include "../signal/signalGraphStructureString.h"
#include "./SignalString.h"
#include "Command.h"

class AudioCommand : public Command
{
public:
    using Command::Command;

protected:
    void describeOutputOptions()
    {
        options.add_options()( "normalize", "Normalize the audio output" )(
            "duration", po::value<std::string>(), "Crop the audio output to a certain duration" )(
            "output-file,o", po::value<std::string>(), "Path to wav file for recording audio\n" )(
            "interactive,i", "Playback in interactive mode" );
    }

    /**
     * Three ways to get a primary input into a file.
     *   1) --input <file>
     *   2) pipe a wav file to stdin
     *   3) use the positional argument
     */
    void descripeInputOptions()
    {
        options.add_options()( "input", po::value<std::string>(), "The main input signal" );
        if ( !stdinIsAPipe() )
            positionalOptions.add( "input", 1 );
    }

    void addWaveformOptions()
    {
        options.add_options()( "waveform",
                               po::value<std::string>()->default_value( "sine" ),
                               "Set the oscillator's waveform (sine|saw|square|triangle)" );
    }

    MonoBuffer* inputWaveform()
    {
        return &Waveforms::byName( args["waveform"].as<std::string>() );
    }

    void addPitchOptions()
    {
        options.add_options()( "pitch",
                               po::value<double>()->default_value( 50 ),
                               "Frequency of the oscillator expressed as a midi pitch number." );
    }

    std::shared_ptr<FrameStream<double>> inputPitch()
    {
        return SignalString::parse( args["pitch"].as<std::string>() );
    }

    std::shared_ptr<NaiveInstruments::Signal<double>> inputFrequency()
    {
        double pitch = args["pitch"].as<double>();
        return NaiveInstruments::SignalShorthands::midiPitch( pitch );
    }

    void addVibratoOptions()
    {
        options.add_options()(
            "vibrato-frequency,vf", po::value<std::string>()->default_value( "5" ), "Vibrato frequency" )(
            "vibrato-amount,va", po::value<std::string>()->default_value( "0" ), "Amount of vibrato (in semitones)" );
    }

    bool interactiveModeEnabled()
    {
        return args.count( "interactive" );
    }

    /**
     * Get the primary audio input as a buffer.
     */
    std::shared_ptr<MonoBuffer> inputAsBuffer()
    {
        if ( args.count( "input" ) )
        {
            const std::string filename = args["input"].as<std::string>();
            return NaiveInstruments::WavReader::readMonoFile( filename.c_str() );
        }
        else if ( stdinIsAPipe() )
        {
            return NaiveInstruments::WavReader::readStdin();
        }
        else
        {
            cerr << "No input!\n";
            throw 1;
        }
    }

    const Hopefully<std::string> outputFile()
    {
        if ( args.count( "output-file" ) )
            return args["output-file"].as<std::string>();
        else
            return Disappointment;
    }

    float outputDuration()
    {
        std::string durationStr = args["duration"].as<std::string>();
        return *Parse::interval( durationStr );
    }

    // TODO: Cleary you are getting confused Joelle
    float inputDuration()
    {
        if ( args.count( "duration" ) )
            return outputDuration();
        else
            return 1;
    }

protected:
    [[deprecated( "Output NaiveInstruments::Signal instead" )]] void output(
        std::shared_ptr<FrameStream<double>> signal )
    {
        auto& path = outputFile();
        if ( stdoutIsAPipe() )
        {
            recordToStdout( signal, outputDuration() );
        }
        else if ( interactiveModeEnabled() )
        {
            InteractivePlayback::play( signal );
        }
        else if ( !path.success() )
            BufferedPlayback::play( signal );
        else
        {
            record( *path, signal, outputDuration() );
            std::cout << *path << "\n";
        }
    }

    void output( NaiveInstruments::SignalShorthands::mono signal )
    {
        std::cout << NaiveInstruments::SignalGraph::signalGraphStructureString( signal ) << "\n";
        auto& path = outputFile();
        if ( stdoutIsAPipe() )
        {
            recordToStdout( signal, outputDuration() );
        }
        else if ( interactiveModeEnabled() )
        {
            // TODO: InteractivePlayback::play( signal );
            std::cerr << "intercative mode not implemented yet for new signals.";
            throw 1;
        }
        else if ( !path.success() )
        {
            BufferedSignalPlayback::play( signal );
        }
        else
        {
            record( *path, signal, outputDuration() );
            std::cout << *path << "\n";
        }
    }
};
