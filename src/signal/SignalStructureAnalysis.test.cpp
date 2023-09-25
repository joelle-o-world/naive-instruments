#include "SignalStructureAnalysis.h"
#include "../test-framework/custom-assertions.h"
#include "SignalShorthands.h"

using namespace NaiveInstruments;
using namespace SignalShorthands;

TEST_CASE( "Can generate structure string for many kinds of signals without crashing" )
{
    auto signal = GENERATE(
        constant( 2 ), sine( 440 ), step_sequence( 60, { sine( 440 ), square( constant( 200 ) * decay( .4 ) ) } ) );
    INFO( typeid( *signal ).name() );
    std::string str = SignalStructureAnalysis::signalGraphStructureString( signal );
}

TEST_CASE( "Generating a structure string for a signal with feedback" )
{
    auto osc           = usaw( 30 );
    osc->frequency     = 150 + 100 * osc;
    auto        signal = sineWavetable( osc );
    std::string graph  = SignalStructureAnalysis::signalGraphStructureString( signal + 2 );
    std::cerr << "GRAPH:\n" << graph << "\n";
}
