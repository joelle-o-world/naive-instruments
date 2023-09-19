#include "signal-processes.h"
#include "../core/MonoBuffer.h"
#include "../test-framework/custom-assertions.h"

using namespace NaiveInstruments;

TEST_CASE( "Creating a vector signal" )
{
    auto buffer    = std::make_shared<std::vector<double>>( 5 );
    ( *buffer )[0] = 5;
    ( *buffer )[1] = 4;
    ( *buffer )[2] = 3;
    ( *buffer )[3] = 2;
    ( *buffer )[4] = 1;


    std::shared_ptr<Signal<double>> signal = std::make_shared<NaiveInstruments::VectorSignal<double>>( buffer );

    CHECK_SIGNAL( signal, { 5, 4, 3, 2, 1 } );
}

TEST_CASE( "signals all have the expected number of inputs" )
{
    auto exampleBuffer = std::make_shared<MonoBuffer>( 10 );
    REQUIRE( USaw().inputs.size() == 1 );
    REQUIRE( Sum<double>().inputs.size() == 2 );
    REQUIRE( Subtract<double>().inputs.size() == 2 );
    REQUIRE( SignFlip().inputs.size() == 1 );
    REQUIRE( Multiply().inputs.size() == 2 );
    REQUIRE( Divide().inputs.size() == 2 );
    REQUIRE( MonoToStereo().inputs.size() == 1 );
    REQUIRE( Repeater<double>().inputs.size() == 1 );
    REQUIRE( Clock<double>().inputs.size() == 0 );
    REQUIRE( Accumulator().inputs.size() == 1 );
    REQUIRE( Modulo().inputs.size() == 2 );
    REQUIRE( Noise().inputs.size() == 0 );

    // TODO: fix causing runtime exception
    /* REQUIRE( VectorSignal<double>( {} ).inputs.size() == 0 ); */

    REQUIRE( Sampler( exampleBuffer ).inputs.size() == 0 );
    REQUIRE( Wavetable( &*exampleBuffer ).inputs.size() == 1 );
    REQUIRE( Wait<double>( 44100 ).inputs.size() == 1 );
    REQUIRE( Skip<double>( 44100 ).inputs.size() == 1 );
    REQUIRE( IntervalToRatio().inputs.size() == 1 );
    REQUIRE( BufferLooper( 44100 ).inputs.size() == 1 );
    REQUIRE( HardClip<double>().inputs.size() == 1 );
}

TEST_CASE( "Creating a stereo vector signal" )
{
    auto buffer    = std::make_shared<std::vector<NaiveInstruments::StereoFrame>>( 5 );
    ( *buffer )[0] = { 5, 0 };
    ( *buffer )[1] = { 4, 1 };
    ( *buffer )[2] = { 3, 2 };
    ( *buffer )[3] = { 2, 3 };
    ( *buffer )[4] = { 1, 4 };


    std::shared_ptr<Signal<NaiveInstruments::StereoFrame>> signal =
        std::make_shared<NaiveInstruments::VectorSignal<NaiveInstruments::StereoFrame>>( buffer );

    CHECK_SIGNAL( signal, { { 5, 0 }, { 4, 1 }, { 3, 2 }, { 2, 3 }, { 1, 4 } } );
}

TEST_CASE( "Creating a Sampler using a contrived MonoBuffer" )
{
    auto        bufferptr = std::make_shared<MonoBuffer>( 10 );
    MonoBuffer& buffer    = *bufferptr;
    buffer[0]             = 5;
    buffer[1]             = 4;
    buffer[2]             = 3;
    buffer[3]             = 2;
    buffer[4]             = 1;

    std::shared_ptr<Signal<double>> sampler = std::make_shared<NaiveInstruments::Sampler>( bufferptr );
    CHECK_SIGNAL( sampler, { 5, 4, 3, 2, 1 } );
}

TEST_CASE( "Constructing a sequence with constants" )
{
    auto mySequence = std::make_shared<Sequence>();
    auto a          = std::make_shared<Constant<double>>();
    a->output       = 1;
    auto b          = std::make_shared<Constant<double>>();
    b->output       = 2;
    mySequence->addStep( 4, a );
    mySequence->addStep( 4, b );

    CHECK_SIGNAL( mySequence, { 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1 } );

    auto c    = std::make_shared<Constant<double>>();
    c->output = 3;
    mySequence->addStep( 3, c );

    CHECK_SIGNAL( mySequence, { 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 1, 1, 1, 1 } );
}
