#include "SignalShorthands.h"
#include "../Waveforms.h"
#include "../test-framework/custom-assertions.h"
#include "Signal.h"
#include <iostream>
#include <memory>
#include <string>

using namespace NaiveInstruments::SignalShorthands;

TEST_CASE( "Using a shorthand to create a constant" )
{
    CHECK_SIGNAL( constant( 10 ), { 10, 10, 10 } );
}


TEST_CASE( "creating a clock" )
{
    CHECK_SIGNAL( t(), { 0, 1, 2, 3, 4, 5 } );
}

TEST_CASE( "Creating an unsigned sawtooth wave" )
{
    CHECK_SIGNAL( usaw( constant( 0 ) ), { 0, 0, 0, 0, 0, 0 } );
    CHECK_SIGNAL( usaw( constant( 1 ) ), { 0, 1.0 / 44100, 2.0 / 44100, 3.0 / 44100 } );
    CHECK_SIGNAL( usaw( constant( 22050 ) ), { 0, .5, 0, .5 } );
}

TEST_CASE( "Adding two clocks together with function" )
{
    CHECK_SIGNAL( add( t(), t() ), { 0, 2, 4, 6 } );
}

TEST_CASE( "Adding together n number of signals using add" )
{
    CHECK_SIGNAL( add( { t(), t(), t() } ), { 0, 3, 6, 9 } );
}

TEST_CASE( "Adding a constant to a clock" )
{
    CHECK_SIGNAL( t() + 10, { 10, 11, 12, 13 } );
}
TEST_CASE( "Adding a clock to a constant" )
{
    CHECK_SIGNAL( 10 + t(), { 10, 11, 12, 13 } );
}

TEST_CASE( "Subtracting one signal from another" )
{
    CHECK_SIGNAL( t() - t(), { 0, 0, 0, 0, 0, 0, 0, 0 } );
}
TEST_CASE( "Subtracting a constant from a signal" )
{
    CHECK_SIGNAL( t() - 10, { -10, -9, -8, -7 } );
}
TEST_CASE( "Subtracting a signal from a constant" )
{
    CHECK_SIGNAL( 10 - t(), { 10, 9, 8, 7, 6 } );
}

TEST_CASE( "negating the sign of a signal" )
{
    CHECK_SIGNAL( -t(), { 0, -1, -2, -3 } );
}

TEST_CASE( "Adding two clocks together with operator overload" )
{

    CHECK_SIGNAL( t() + t(), { 0, 2, 4, 6 } );
}

TEST_CASE( "Multiplying two signals together" )
{
    CHECK_SIGNAL( t() * t(), { 0, 1, 4, 9, 16 } );
}

TEST_CASE( "Multiply a signal by a constant" )
{
    CHECK_SIGNAL( t() * 2, { 0, 2, 4, 6, 8 } );
}
TEST_CASE( "multiplying a constant by a signal" )
{
    CHECK_SIGNAL( 3 * t(), { 0, 3, 6, 9, 12 } );
}
TEST_CASE( "dividing one signal by another" )
{
    CHECK_SIGNAL( t() / ( t() + 1 ), { 0, 1.0 / 2, 2.0 / 3, 3.0 / 4 } );
}
TEST_CASE( "dividing a signal by a consntant" )
{
    CHECK_SIGNAL( t() / 2, { 0, .5, 1, 1.5, 2 } );
}
TEST_CASE( "dividing a constant by a signal" )
{
    CHECK_SIGNAL( 1 / ( t() + 1 ), { 1.0 / 1, 1 / 2.0, 1 / 3.0, 1 / 4.0 } );
}


TEST_CASE( "Creating a sampler with a contrived buffer using a shorthand" )
{
    auto        bufferptr = std::make_shared<MonoBuffer>( 10 );
    MonoBuffer& buffer    = *bufferptr;
    buffer[0]             = 5;
    buffer[1]             = 4;
    buffer[2]             = 3;
    buffer[3]             = 2;
    buffer[4]             = 1;

    auto signal = sampler( bufferptr );
    CHECK_SIGNAL( signal, { 5, 4, 3, 2, 1 } );
}

TEST_CASE( "Creating a sampler using an audio file" )
{
    auto buffer = NaiveInstruments::WavReader::readMonoFile( "audio-source-files/piano.wav" );
    auto signal = sampler( buffer );
    referenceToneTest( "piano sample", signal, 1 );
}

TEST_CASE( "unsigned sawtooth waves" )
{
    CHECK_SIGNAL( usaw( constant( 4410 ) ), { 0, .1, .2, .3 } );
}

TEST_CASE( "Constructing a sineWavetable" )
{
    auto table = NaiveInstruments::SignalShorthands::sineWavetable( usaw( constant( 1 ) ) );

    // TODO: Hard to write a test now..
    table->sync( 1 );
    CHECK( table->output == sin( M_PI * 2 * 1 / 44100.0 ) );
}

TEST_CASE( "shorthand for sine waves" )
{
    auto signal = sine( constant( 1 ) );
    CHECK_FRAME( signal, 1, sin( M_PI * 2 * 1 / 44100.0 ) );
    referenceToneTest( "440Hz Sine", sine( constant( 440 ) ), .1 );
}

TEST_CASE( "short hand for square waves" )
{
    auto signal = square( constant( 1 ) );
    CHECK_FRAME( signal, 1, 1 );
    CHECK_FRAME( signal, 44099, -1.0 );
    CHECK_FRAME( signal, 44101, 1.0 );
}

TEST_CASE( "checksum test for square wave" )
{
    referenceToneTest( "440Hz square wave", square( constant( 44 ) ), .1 );
}

// TODO: test saw()
TEST_CASE( "shorthand for triangle waves" )
{
    auto signal = triangle( constant( 44100.0 / 4 ) );
    CHECK_SIGNAL( signal, { 0, 1, 0, -1, 0 } );
}

TEST_CASE( "The first frame in the triangle waveform should be 0" )
{
    auto signal = triangleWavetable( constant( 0 ) );
    CHECK_FRAME( signal, 1, 0 );
    CHECK( Waveforms::triangle()[0] == 0 );
}
// TODO: test oscWithWavetableFromFile()

TEST_CASE( "shorthand for lfo" )
{
    auto signal = lfo( constant( 10 ), constant( 2 ), constant( 44100.0 / 4 ) );
    CHECK_SIGNAL( signal, { 10, 12, 10, 8, 10 } );
}

TEST_CASE( "Waiting before starting a signal" )
{
    // Oh dear, mixing units of seconds and samples even in the first unit test
    CHECK_SIGNAL( waitSamples( 5, triangle( constant( 44100.0 / 4.0 ) ) ), { 0, 0, 0, 0, 0, 0, 1, 0, -1, 0 } );

    auto signal = waitSeconds( 1, square( constant( 1 ) ) );
    CHECK_FRAME( signal, 0, 1 );
    CHECK_FRAME( signal, 44100 - 1, 1 );
    CHECK_FRAME( signal, 44100, 1 );
    CHECK_FRAME( signal, 44100 * 1.5 + 1, -1 );
}

TEST_CASE( "Skipping the start of signals" )
{
    CHECK_SIGNAL( skipSamples( 5, t() ), { 5, 6, 7, 8, 9, 10 } );

    CHECK_SIGNAL( skip( 1, t() ), { 44100, 44101, 44102, 44103, 44104, 44105 } );
}

TEST_CASE( "Elapsing a signal" )
{
    CHECK_SIGNAL( elapseFrames( t(), 5 ), { 0, 1, 2, 3, 4, 4, 4, 4, 4 } );
}

TEST_CASE( "Slicing a signal" )
{
    CHECK_SIGNAL( slice( t(), .01, .02 ), { 441, 442, 443 } );
}

TEST_CASE( "Converting intervals to frequency ratios" )
{
    CHECK_SIGNAL( interval( constant( 7 ) ), { pow( 2, 7 / 12.0 ), pow( 2, 7 / 12.0 ) } );
    CHECK_SIGNAL( interval( constant( 0 ) ), { 1, 1, 1, 1, 1 } );
    CHECK_SIGNAL( interval( constant( 12 ) ), { 2, 2, 2, 2, 2, 2, 2 } );
}

TEST_CASE( "Using a buffer loop" )
{
    CHECK_SIGNAL( bufferLoopInSamples( t(), 5 ), { 0, 1, 2, 3, 4, 0, 1, 2, 3, 4 } );
    CHECK_SIGNAL( fixedLoop( t(), 5 / 44100.0 ), { 0, 1, 2, 3, 4, 0, 1, 2, 3, 4 } );
}

TEST_CASE( "Hard clipping a mono signal" )
{
    CHECK_SIGNAL( clip( t() / 4 ), { 00, .25, .5, .75, 1, 1, 1, 1 } );
    CHECK_SIGNAL( clip( -t() / 4 ), { 00, -.25, -.5, -.75, -1, -1, -1, -1 } );
}

TEST_CASE( "Delaying a signal by fixed duration" )
{
    CHECK_SIGNAL( fixed_sample_delay( 10, t() ), { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } );

    auto delayedSignal = fixed_delay( .01, t() );
    CHECK_FRAME( delayedSignal, 0, 0 );
    CHECK_FRAME( delayedSignal, 440, 0 );
    CHECK_FRAME( delayedSignal, 441, 0 );
    CHECK_FRAME( delayedSignal, 442, 1 );
    CHECK_FRAME( delayedSignal, 443, 2 );
}

TEST_CASE( "harmonic_series(fundamental, numberOfHarmonics)" )
{
    auto freq = GENERATE( range( 100, 1000, 50 ) );
    referenceToneTest(
        "harmonic_series( " + std::to_string( freq ) + ", 4)", harmonic_series( constant( freq ), 4 ), .5 );
}

TEST_CASE( "harmonic_spread()" )
{
    auto fundamental       = GENERATE( 200, 20, 50, 440, 30, 40 );
    auto numberOfHarmonics = GENERATE( 9, 17, 65 );
    auto step_duration     = .1;
    referenceToneTest( std::to_string( numberOfHarmonics ) + " harmonic spread on " + std::to_string( fundamental )
                           + "Hz",
                       harmonic_spread( fundamental, numberOfHarmonics, step_duration ),
                       2 );
}

TEST_CASE( "ramp as an envelope" )
{
    referenceToneTest( "440Hz Sine boop", sine( constant( 440 ) ) * ramp( 1, 1, 0 ), 1 );
}

TEST_CASE( "Linear decay" )
{
    referenceToneTest( "Sine with linear decay envelope on amplitude", sine( 100 ) * decay( 2 ), 2 );
}

TEST_CASE( "Frequency modulation in series" )
{
    referenceToneTest( "440 into 220 fm series", fm_series( { constant( 440 ), constant( 6 ), constant( 220 ) } ), 1 );
}

TEST_CASE( "Various signals are resettable and repeat the same sequence" )
{
    std::vector<mono> signalsToTest = { t(),
                                        constant( 1 ),
                                        noise(),
                                        sine( 440 ),
                                        fm_series( { constant( 440 ), constant( 6 ), constant( 220 ) } ),
                                        sine( 100 ) * decay( 2 ),
                                        wait( 100, sine( 440 ) )

    };
    // TODO: Resetting signals which include feed back

    for ( int i = 0; i < signalsToTest.size(); ++i )
    {
        auto signal = signalsToTest[i];
        INFO( typeid( *signal ).name() );
        int                                    numberOfFramesToCheck = 100;
        NaiveInstruments::SignalReader<double> reader;
        double                                 firstPass[numberOfFramesToCheck];
        reader = signal;
        for ( int frame = 0; frame < numberOfFramesToCheck; ++frame )
            firstPass[frame] = reader[frame];
        for ( int frame = 0; frame < numberOfFramesToCheck; ++frame )
            CHECK( reader[frame] == firstPass[frame] );
    }
}


TEST_CASE( "super saws!" )
{
    referenceToneTest( "super saw", supersaw( ramp( 60, 1, 72 ), constant( 10 ) ), 1 );
    referenceToneTest(
        "super saw chord", super_saws( { constant( 40 ), constant( 43 ), constant( 47 ) }, constant( .5 ) ), 1 );
    referenceToneTest( "sliding super saw chord", super_saws( minor_triad( ramp( 40, 1, 28 ) ), constant( .5 ) ), 5 );
    referenceToneTest( "sliding super saw chord 2", super_saws( minor_triad( ramp( 52, 1, 40 ) ), constant( .5 ) ), 5 );
}

// TODO: Test hard clipping stereo signals
/* TEST_CASE( "Hard clipping a stereo signal" ) */
/* { */
/*     CHECK_SIGNAL( clip( vectorSignal<StereoFrame>( { { 0, 0 }, { 0, 10 }, { -10, 0 } } ) ), */
/*                   { { 0, 0 }, { 0, 1 }, { 0, -1 } } ); */
/* } */

// TODO: Test Wait with stereo signals

TEST_CASE( "Creating a step sequencer" )
{
    auto sequence = step_sequence( 60, { constant( 69 ), constant( 420 ), nullptr, constant( 0 ) } );
    CHECK_FRAME( sequence, 0 * 44100, 69 );
    CHECK_FRAME( sequence, 1 * 44100, 420 );
    CHECK_FRAME( sequence, 2 * 44100, 420 );
    CHECK_FRAME( sequence, 3 * 44100, 0 );
}

// TODO: Test repeatedSound

TEST_CASE( "Panning a signal right to left using a control signal" )
{

    referenceToneTest( "panning sine wave", pan( sine( 440 ), triangle( constant( 2 ) ) ), 2 );
}

TEST_CASE( "Selecting channels from stereo signals" )
{
    CHECK_SIGNAL( left( stereoChannels( t(), -t() ) ), { 0, 1, 2, 3, 4, 5 } );
    CHECK_SIGNAL( right( stereoChannels( t(), -t() ) ), { 0, -1, -2, -3, -4, -5 } );
}
