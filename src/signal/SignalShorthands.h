#pragma once

#include "../Waveforms.h"
#include "./signal-processes.h"
#include "Signal.h"
#include <ctime>
#include <memory>

namespace NaiveInstruments
{
namespace SignalShorthands
{

typedef std::shared_ptr<Signal<double>>      mono;
typedef std::shared_ptr<Signal<StereoFrame>> stereo;

/**
 * Create a `Constant` signal - Every frame is `value`.
 */
inline mono constant( double value )
{
    auto signal    = std::make_shared<Constant<double>>();
    signal->output = value;
    return signal;
}

/**
 * Constant stereo signal (every frame the same)
 */
inline stereo constant( double left, double right )
{
    auto signal          = std::make_shared<Constant<StereoFrame>>();
    signal->output.left  = left;
    signal->output.right = right;
    return signal;
}

/**
 * Create a `Clock` signal - writes the current frame index into each frame.
 */
inline mono t()
{
    return std::make_shared<Clock<double>>();
}

/**
 * Create a `USaw` unsigned sawtooth wave signal with the given `frequency`.
 * Slides from 0 to 1 `frequency` times per second.
 */
inline mono usaw( mono frequency )
{
    auto saw       = std::make_shared<USaw>();
    saw->frequency = frequency;
    return saw;
}

/**
 * Add two signals together. Creates an `Add` process adding together `a` and
 * `b`.
 */
inline mono add( mono a, mono b )
{
    auto adder    = std::make_shared<Sum<double>>();
    adder->input1 = a;
    adder->input2 = b;
    return adder;
}

/**
 * Add any number of signals together.
 */
inline mono add( std::vector<mono> signals )
{
    mono total = signals[0];
    for ( int i = 1; i < signals.size(); ++i )
        total = add( total, signals[i] );
    return total;
}

/**
 * Add two signals together using the `+` operator.
 * Alias for `add(a, b)`
 */
inline mono operator+( mono a, mono b )
{
    return add( a, b );
}

/**
 * Add a constant value to a signal.
 */
inline mono operator+( mono a, double b )
{
    return add( a, constant( b ) );
}

/**
 * Add a signal to a constant value.
 */
inline mono operator+( double a, mono b )
{
    return add( constant( a ), b );
}

/**
 * Add a signal to a SignalReader in place
 */
inline void operator+=( SignalReader<double>& a, mono b )
{
    a = a.ptr + b;
}

/**
 * Subtract one signal from another. Creates a `Subtract` signal process
 * subtracting `b` from `a`.
 */
inline mono subtract( mono a, mono b )
{
    auto subtracter = std::make_shared<NaiveInstruments::Subtract<double>>();
    subtracter->a   = a;
    subtracter->b   = b;
    return subtracter;
}

/**
 * Subtract one signal from another.
 * Alias for `subtract(a,b)`
 */
inline mono operator-( mono a, mono b )
{
    return subtract( a, b );
}

/**
 * Subtract a constant value from a signal.
 */
inline mono operator-( mono a, double b )
{
    return subtract( a, constant( b ) );
}

/**
 * Subtract a signal from a constant value.
 */
inline mono operator-( double a, mono b )
{
    return subtract( constant( a ), b );
}

/**
 * Flip the sign on a signal. Creates a `SignFlip` signal process flipping the
 * sign on `a`.
 */
inline mono operator-( mono a )
{
    auto flip   = std::make_shared<SignFlip>();
    flip->input = a;
    return flip;
}

/**
 * Multiply two signals together. Creates a `Multiply` signal process
 * multiplying `a` and `b`.
 */
inline mono multiply( mono a, mono b )
{
    auto multiplied = std::make_shared<NaiveInstruments::Multiply>();
    multiplied->a   = a;
    multiplied->b   = b;
    return multiplied;
}

/**
 * Multiply two signals together using the `*` operator.
 * alias for `multiply(a,b)`.
 */
inline mono operator*( mono a, mono b )
{
    return multiply( a, b );
}

/**
 * Multiply a signal by a constant value.
 */
inline mono operator*( mono a, double b )
{
    return a * constant( b );
}

/**
 * Multiply a constant value by a signal.
 */
inline mono operator*( double a, mono b )
{
    return constant( a ) * b;
}

/**
 * Divide one signal by another. Creates a `Divide` signal process.
 */
inline mono divide( mono numerator, mono denominator )
{
    auto divided         = std::make_shared<NaiveInstruments::Divide>();
    divided->numerator   = numerator;
    divided->denominator = denominator;
    return divided;
}

/**
 * Divide one signal by another using the `/` operator.
 * Alias for `divide(numerator, denominator)`
 */
inline mono operator/( mono numerator, mono denominator )
{
    return divide( numerator, denominator );
}

/**
 * Divide a signal by a constant value.
 */
inline mono operator/( mono numerator, double denominator )
{
    return numerator / constant( denominator );
}

/**
 * Divide a constant value by a signal.
 */
inline mono operator/( double numerator, mono denominator )
{
    return constant( numerator ) / denominator;
}

/**
 * Sum a vector of signals together, applying attenuation to avoid clipping.
 * In other words, take the mean average (frame-by-frame) of a vector of signals.
 */
inline mono mix( std::vector<mono> signals )
{
    return add( signals ) / signals.size();
}

/**
 * Create a seeded white noise signal.
 */
inline mono noise( uint64_t seed = 1 )
{
    return std::make_shared<Noise>( seed );
}


/**
 * Create a sampler playing the given MonoBuffer sample.
 */
inline mono sampler( std::shared_ptr<MonoBuffer> buffer )
{
    // TODO: How do we feel about the raw pointer here?
    return std::make_shared<NaiveInstruments::Sampler>( buffer );
}


/**
 * Create a wavetable using the given `buffer` as a wavetable. The `phase`
 * signal is used to select which frames are read from the wavetable and
 * written to the output.
 */
inline mono wavetable( MonoBuffer* buffer, mono phase )
{
    // TODO: How do we feel about the raw pointer here?
    auto signal   = std::make_shared<NaiveInstruments::Wavetable>( buffer );
    signal->phase = phase;
    return signal;
}

/**
 * Use an audio file as a wavetable. The `phase` signal is used to select which
 * frames are read from the wavetable and written to the output.
 */
inline mono wavetableFromFile( const std::string& filePath, mono phase )
{
    return wavetable( &Waveforms::fromFile( filePath ), phase );
}

/**
 * Create a wavetable for a square wave, using the `phase` lookup signal.
 */
inline mono squareWavetable( mono phase )
{
    return wavetable( &Waveforms::square(), phase );
}

/**
 * Create a wavetable for a sine wave, using the `phase` lookup signal.
 */
inline mono sineWavetable( mono phase )
{
    return wavetable( &Waveforms::sine(), phase );
}

/**
 * Create a wavetable for a triangle wave, using the `phase` lookup signal.
 */
inline mono triangleWavetable( mono phase )
{
    return wavetable( &Waveforms::triangle(), phase );
}

/**
 * Create a square wave using the given `frequency` control signal.
 */
inline mono square( mono frequency )
{
    return wavetable( &Waveforms::square(), usaw( frequency ) );
}

/**
 * Create a sine wave using the given `frequency` control signal.
 */
inline mono sine( mono frequency )
{
    return wavetable( &Waveforms::sine(), usaw( frequency ) );
}

/**
 * Create a sine wave with a constant `frequency`
 */
inline mono sine( double frequency )
{
    return sine( constant( frequency ) );
}

/**
 * Create a saw wave with the given `frequency` control signal.
 */
inline mono saw( mono frequency )
{
    return wavetable( &Waveforms::saw(), usaw( frequency ) );
}

/**
 * Create a triangle wave with the given `frequency` control signal.
 */
inline mono triangle( mono frequency )
{
    return wavetable( &Waveforms::triangle(), usaw( frequency ) );
}


/**
 * Create an oscillator with the given `frequency` control signal using a
 * wavetable from an audio file.
 */
inline mono oscWithWavetableFromFile( const std::string& filePath, mono frequency )
{
    return wavetableFromFile( filePath, usaw( frequency ) );
}

/**
 * Create an oscillator using the given buffer as a wavetable".
 */
// TODO: use smart pointer
inline mono oscWithWavetable( MonoBuffer* table, mono frequency )
{
    return wavetable( table, usaw( frequency ) );
}

/**
 * Create a low frequency oscillator (LFO) control signal which osciallates
 * ± `range` around a `centre` control signal at `frequency` times per second.
 */
inline mono lfo( mono centre, mono range, mono frequency = constant( 5 ) )
{
    return centre + triangle( frequency ) * range;
}

/**
 * Create a signal which waits the given number of frames before writing the
 * `input` (from its beginning).
 */
inline mono waitSamples( int waitTimeInSamples, mono input )
{
    auto signal   = std::make_shared<Wait<double>>( waitTimeInSamples );
    signal->input = input;
    return signal;
}

/**
 * Create a signal which waits the given number of seconds before writing the
 * `input` (from its beginning).
 */
inline mono waitSeconds( double waitTimeInSeconds, mono input )
{
    return waitSamples( waitTimeInSeconds * 44100, input );
}

/**
 * Alias for wait seconds.
 */
inline mono wait( double waitTimeInSeconds, mono input )
{
    return waitSeconds( waitTimeInSeconds, input );
}


/**
 * Skip for first `skipTimeInSamples` samples of the `input` signal.
 */
inline mono skipSamples( int skipTimeInSamples, mono input )
{
    auto signal   = std::make_shared<Skip<double>>( skipTimeInSamples );
    signal->input = input;
    return signal;
}

/**
 * Skip for first `skipTimeInSamples` seconds of the `input` signal.
 */
inline mono skipSeconds( double skipTimeInSeconds, mono input )
{
    return skipSamples( skipTimeInSeconds * 44100, input );
}

/**
 * Skip for first `skipTimeInSamples` seconds of the `input` signal.
 * Alias for `skipSeconds`
 */
inline mono skip( double skipTimeInSeconds, mono input )
{
    return skipSeconds( skipTimeInSeconds, input );
}

/**
 * Play only the first `numberOfFrames` frames of a signal, then repeat the
 * last frame ad infinitum.
 */
inline mono elapseFrames( mono input, int numberOfFrames )
{
    auto signal   = std::make_shared<Elapse<double>>( numberOfFrames );
    signal->input = input;
    return signal;
}

/**
 * Play only the first `duration` seconds of a signal, then repeat the
 * last frame ad infinitum.
 */
inline mono elapseSeconds( mono input, double duration )
{
    return elapseFrames( input, duration * 44100 );
}


/**
 * Play only the first `duration` seconds of a signal, then repeat the
 * last frame ad infinitum.
 * Alias for `elapseSeconds`.
 */
inline mono elapse( mono input, double duration )
{
    return elapseSeconds( input, duration );
}

/**
 * Play a portion of the `input` signal starting from `to` seconds, ending at
 * `from` seconds.
 */
inline mono slice( mono input, double from, double to )
{
    // TODO: Could it handle negative duration?
    return elapse( skip( from, input ), to - from );
}

/**
 * Convert the given `interval` control signal (in semitones) to a frequency ratio control signal.
 */
inline mono interval( mono interval )
{
    auto signal      = std::make_shared<IntervalToRatio>();
    signal->interval = interval;
    return signal;
}

/**
 * Convert the given midi `pitch` control signal into a frequency control signal.
 */
inline mono midiPitch( mono pitch )
{
    return 440 * interval( pitch - 69 );
}

inline mono midiPitch( double pitch )
{
    return midiPitch( constant( pitch ) );
}

/**
 * Loop the first `loopDurationInSamples` frames of the `input` signal using a memory buffer.
 */
inline mono bufferLoopInSamples( mono input, int loopDurationInSamples )
{
    auto looper   = std::make_shared<BufferLooper>( loopDurationInSamples );
    looper->input = input;
    return looper;
}

/**
 * Loop the first `loopDurationInSeconds` seconds of the `input` signal using a memory buffer.
 */
inline mono fixedLoop( mono input, double loopDurationInSeconds )
{
    return bufferLoopInSamples( input, loopDurationInSeconds * 44100 );
}

/**
 * Clip the `input` signal within the range -1.0 to 1.0
 */
inline mono clip( mono input )
{
    auto clipper   = std::make_shared<HardClip<double>>();
    clipper->input = input;
    return clipper;
}

/**
 * Linear crossfade (no law) from `before` signal to `after` signal over the
 * course of `duration` seconds.
 */
inline mono ramp( mono before, mono duration, mono after )
{
    auto signal      = std::make_shared<LinearRamp>();
    signal->before   = before;
    signal->duration = duration;
    signal->after    = after;
    return signal;
}


/**
 * Linear crossfade (no law) from `before` signal to `after` signal over the
 * course of `duration` seconds.
 */
inline mono ramp( double before, mono duration, double after )
{
    return ramp( constant( before ), duration, constant( after ) );
}

/**
 * Linear ramp from `before` to `after` over `duration seconds
 */
inline mono ramp( double before, double duration, double after )
{

    return ramp( constant( before ), constant( duration ), constant( after ) );
}

/**
 * Linear attack envelope (from 0.0 to 1.0) lasting `durition` seconds.
 */
inline mono attack( mono duration )
{
    return ramp( 0, duration, 1 );
}

/**
 * Linear attack envelope (from 0.0 to 1.0) lasting `durition` seconds.
 */
inline mono attack( double duration )
{
    return ramp( 0, duration, 1 );
}

/**
 * Linear decay envelope (from 1.0 to 0.0) lasting `duration` seconds.
 */
inline mono decay( mono duration )
{
    return ramp( constant( 1 ), duration, constant( 0 ) );
}

/**
 * Linear decay envelope (from 1.0 to 0.0) lasting `duration` seconds.
 */
inline mono decay( double duration )
{
    return decay( constant( duration ) );
}

/**
 * Duplicate a mono signal on both channels of a stereo signal.
 */
inline stereo monoToStereo( mono input )
{
    auto signal   = std::make_shared<MonoToStereo>();
    signal->input = input;
    return signal;
}

/**
 * Combine `left` and `right` mono signals into a stereo signal.
 */
inline stereo stereoChannels( mono left, mono right )
{
    auto signal   = std::make_shared<StereoChannels>();
    signal->left  = left;
    signal->right = right;
    return signal;
}

/**
 * Pan audio to left & right channels
 */
inline stereo pan( stereo input, mono pan )
{
    auto signal   = std::make_shared<Pan>();
    signal->input = input;
    signal->pan   = pan;
    return signal;
}

inline stereo pan( stereo input, double panAmount )
{
    return pan( input, constant( panAmount ) );
}

inline stereo pan( mono input, mono panValue )
{
    return pan( monoToStereo( input ), panValue );
}

inline stereo pan( mono input, double panAmount )
{
    return pan( input, constant( panAmount ) );
}


/**
 * Hard clip a stereo 'input` signal within the range -1.0 to 1.0
 */
inline stereo clip( stereo input )
{
    auto clipper   = std::make_shared<HardClip<StereoFrame>>();
    clipper->input = input;
    return clipper;
}

/**
 * Performing additive synthesis using sine waves that are harmonics of the
 * `fundamental` frequency control signal.
 */
inline mono harmonic_series( mono fundamental, int numberOfHarmonics )
{
    mono signal = sine( fundamental );
    for ( int i = 1; i < numberOfHarmonics; ++i )
    {
        signal = signal + sine( fundamental * i );
    }
    return signal / numberOfHarmonics;
}

/**
 * Performing additive synthesis using sine waves that are harmonics of the
 * `fundamental` frequency. The onsets of the harmonics are staggered to create
 * an arpegiated "spread" chord effect.
 */
inline mono harmonic_spread( double fundamental, int numberOfHarmonics, float step_duration )
{
    mono signal = sine( fundamental );
    for ( int i = 0; i < numberOfHarmonics; ++i )
    {
        signal = signal + wait( step_duration * i, sine( fundamental * i ) );
    }
    return signal / numberOfHarmonics;
}

/**
 * Perform frequency modulation ("FM") synthesis with the oscillators in parallel.
 *
 * i.e. osc1 modulates osc2 modulase osc3 which is the output (carrier) signal.
 */
inline mono fm_series(
    /**
     * Interleaved {frequency, feedforward, frequency, ... etc} control signals couplets.
     */
    std::vector<mono> frequencyFeedforwards )
{
    mono current = sine( frequencyFeedforwards[0] );
    for ( int i = 1; i < frequencyFeedforwards.size(); ++i )
    {
        if ( i % 2 == 0 )
        {
            // frequency
            current = sine( interval( current ) * frequencyFeedforwards[i] );
        }
        else
        {
            // feedfoward
            current = current * frequencyFeedforwards[i];
        }
    }
    return current;
}

/**
 * Delay signal by a fixed duration using a sample buffer.
 */
inline mono fixed_sample_delay( int durationInSamples, mono input )
{
    auto delayedSignal   = std::make_shared<FixedDelay>( durationInSamples );
    delayedSignal->input = input;
    return delayedSignal;
}

/**
 * Delay signal by a fixed duration using a sample buffer.
 */
inline mono fixed_delay( double durationInSeconds, mono input )
{
    return fixed_sample_delay( durationInSeconds * 44100, input );
}

/**
 * Evenly spaced sawtooth waves all summed together for a dramatic sound.
 */
inline mono supersaw( mono pitch, mono width, int numberOfOscs = 3 )
{
    // TODO: Add options for other waveforms
    // TODO: Add rall off
    auto              spacing = width / numberOfOscs;
    auto              bottom  = pitch - width / 2;
    std::vector<mono> oscs;
    for ( int i = 0; i < numberOfOscs; ++i )
    {
        oscs.push_back( saw( midiPitch( pitch + width * ( float( i ) / numberOfOscs - .5 ) ) ) );
    }
    return mix( oscs );
}

inline mono super_saws( std::vector<mono> pitches, mono width, int numberOfOscs = 3 )
{
    std::vector<mono> saws;
    for ( auto pitch : pitches )
    {
        saws.push_back( supersaw( pitch, width, numberOfOscs ) );
    }

    return mix( saws );
}

inline std::vector<mono> minor_triad( mono rootNote )
{
    return { rootNote, rootNote + 3, rootNote + 7 };
}


// TODO: /* inline mono fm( std::vector<std::vector<mono>> rows ) */


// TODO: inline mono fm( mono fundamental, std::vector<mono> ratios, std::vector<mono> interactions );

inline std::shared_ptr<Sequence> emptySequence()
{
    return std::make_shared<Sequence>();
}
/**
 * Create a sequence using steps of equal duration. Use `nullptr` to extend the duration of the previous step.
 */
inline mono step_sequence( double stepsPerMinute, std::vector<mono> steps )
{
    double step_duration = 60.0 / stepsPerMinute * 44100;
    auto   sequence      = emptySequence();
    for ( auto step : steps )
    {
        if ( step == nullptr && sequence->numberOfSteps() )
            sequence->lastStep()->duration += step_duration;
        else
            sequence->addStep( step_duration, step );
    }
    return sequence;
}

inline mono repeatedSound( mono sound, std::vector<float> durationsInSeconds )
{
    auto sequence = emptySequence();
    for ( auto duration : durationsInSeconds )
    {
        sequence->addStep( duration * 44100, sound );
    }
    return sequence;
}

/**
 * Isolate the left hand channel of a stereo signal.
 */
inline mono left( stereo input )
{
    auto signal   = std::make_shared<SelectLeftChannel>();
    signal->input = input;
    return signal;
}

/**
 * Isolate the right channel of a stereo signal.
 */
inline mono right( stereo input )
{
    auto signal   = std::make_shared<SelectRightChannel>();
    signal->input = input;
    return signal;
}

// TODO: ratioToInterval
// TODO: frequencyToMidiPitch

}; // namespace SignalShorthands
} // namespace NaiveInstruments
