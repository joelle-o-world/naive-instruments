#include "SignalShorthands.h"
#include "../Waveforms.h"
#include "../test-framework/custom-assertions.h"
#include "Signal.h"
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
    MonoBuffer buffer( 10 );
    buffer[0] = 5;
    buffer[1] = 4;
    buffer[2] = 3;
    buffer[3] = 2;
    buffer[4] = 1;

    auto signal = sampler( &buffer );
    CHECK_SIGNAL( signal, { 5, 4, 3, 2, 1 } );
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
    AUDIO_TEST( "440Hz Sine", sine( constant( 440 ) ), .1 );
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
    AUDIO_TEST( "440Hz square wave", square( constant( 44 ) ), .1 );
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

TEST_CASE( "signal reader += operator" )
{
    auto                                   fundamental         = constant( 200 );
    int                                    numberOfHarmonics   = 17;
    double                                 arpegiationInterval = .2;
    NaiveInstruments::SignalReader<double> reader;
    reader = sine( fundamental );
    for ( int i = 1; i < numberOfHarmonics; ++i )
    {
        reader += wait( i * arpegiationInterval, sine( fundamental * i ) );
    }
    AUDIO_TEST(
        "17 harmonic series on 200Hz", reader.ptr / numberOfHarmonics, numberOfHarmonics * arpegiationInterval + 1.6 );
}

TEST_CASE( "harmonic_series(fundamental, numberOfHarmonics)" )
{
    auto freq = GENERATE( range( 100, 1000, 50 ) );
    AUDIO_TEST( "harmonic_series( " + std::to_string( freq ) + ", 4)", harmonic_series( constant( freq ), 4 ) );
}

TEST_CASE( "harmonic_spread()" )
{
    auto fundamental       = GENERATE( 200, 20, 50, 440, 30, 40 );
    auto numberOfHarmonics = GENERATE( 9, 17, 65 );
    auto step_duration     = .1;
    AUDIO_TEST( std::to_string( numberOfHarmonics ) + " harmonic spread on " + std::to_string( fundamental ) + "Hz",
                harmonic_spread( constant( fundamental ), numberOfHarmonics, step_duration ),
                2 * numberOfHarmonics * step_duration );
}

TEST_CASE( "ramp as an envelope" )
{
    AUDIO_TEST( "440Hz Sine boop", sine( constant( 440 ) ) * ramp( 1, 1, 0 ), 1 );
}

TEST_CASE( "sliding between pitches" )
{
    AUDIO_TEST( "pitch slide 1", sine( pitch_slide( 10, 80, 1 ) ), 1 );
}

TEST_CASE( "Linear decay" )
{
    AUDIO_TEST( "Sine with linear decay envelope on amplitude", sine( 100 ) * decay( 2 ), 2 );
}

TEST_CASE( "Frequency modulation in series" )
{
    AUDIO_TEST( "440 into 220 fm series", fm_series( { constant( 440 ), constant( 6 ), constant( 220 ) } ), 1 );
}


// TODO: Test hard clipping stereo signals
/* TEST_CASE( "Hard clipping a stereo signal" ) */
/* { */
/*     CHECK_SIGNAL( clip( vectorSignal<StereoFrame>( { { 0, 0 }, { 0, 10 }, { -10, 0 } } ) ), */
/*                   { { 0, 0 }, { 0, 1 }, { 0, -1 } } ); */
/* } */

// TODO: Test Wait with stereo signals
