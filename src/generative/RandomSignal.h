#include "../signal/SignalShorthands.h"
#include "Random.h"
#include <functional>
#include <map>

namespace NaiveInstruments
{

typedef std::function<std::shared_ptr<Signal<double>>()> Generator;

class RandomSignal
{

    std::map<std::string, Generator> map;

    Random random;

public:
    RandomSignal( uint64_t seed = Random::clockSeed() )
    : random( seed )
    {
        using namespace NaiveInstruments::SignalShorthands;
        std::cerr << "Random seed = " << seed << "\n";
        // sounds
        map["sound"] = [this]() { return create( random( { "tone", "boop", "chaotic" } ) ); };
        map["tone"]  = [this]() {
            return create( random( { "sine_wave", "square_wave", "sawtooth_wave", "triangle_wave" } ) );
        };
        map["sine_wave"]     = [this]() { return sine( create( "frequency" ) ); };
        map["square_wave"]   = [this]() { return square( create( "frequency" ) ); };
        map["sawtooth_wave"] = [this]() { return saw( create( "frequency" ) ); };
        map["triangle_wave"] = [this]() { return triangle( create( "frequency" ) ); };
        map["chaotic"]       = [this]() {
            return create( random( { "chaotic_sine", "chaotic_saw", "chaotic_square", "chaotic_triangle" } ) );
        };
        map["chaotic_sine"]     = [this]() { return sine( create( "worm_frequency" ) ); };
        map["chaotic_saw"]      = [this]() { return saw( create( "worm_frequency" ) ); };
        map["chaotic_square"]   = [this]() { return square( create( "worm_frequency" ) ); };
        map["chaotic_triangle"] = [this]() { return triangle( create( "worm_frequency" ) ); };


        map["boop"]      = [this]() { return decay( create( "duration" ) ) * create( "tone" ); };
        map["sine_boop"] = [this]() { return decay( create( "duration" ) ) * create( "tone" ); };

        // Control signals
        map["duration"]        = [this]() { return constant( random( .1, 4 ) ); };
        map["attack_envelope"] = [this]() { return attack( create( "duration" ) ); };
        map["decay_envelope"]  = [this]() { return decay( create( "duration" ) ); };
        map["frequency"]       = [this]() { return midiPitch( create( "pitch" ) ); };
        map["low_frequency"]   = [this]() { return constant( random( .1, 10 ) ); };
        map["constant_pitch"]  = [this]() { return constant( random( 20, 108 ) ); };
        map["sliding_pitch"] = [this]() { return ramp( create( "pitch" ), create( "duration" ), create( "pitch" ) ); };
        map["pitch"]         = [this]() { return create( random( { "constant_pitch", "sliding_pitch" } ) ); };
        map["lfo"]           = [this]() { return lfo( constant( 0 ), constant( 1 ), create( "low_frequency" ) ); };
        map["worm"]          = [this]() {
            auto signal = create( "lfo" );
            int  count  = 1;
            while ( random.boolean( .8 ) )
            {
                ++count;
                signal = signal + create( "lfo" );
            }
            return signal / count;
        };
        map["worm_frequency"] = [this]() { return midiPitch( 50 + 30 * create( "worm" ) ); };


        map["kick_envelope"] = [this]() {
            double settle       = random( 40, 60 );
            mono   settle_drift = random.boolean( .5 ) ? interval( drift( random( -6, 6 ) ) ) : constant( 1 );

            auto attack =
                random.boolean( .5 ) ? interval( noise() * ramp( random( 64 ), random( .25 ), 0 ) ) : constant( 1 );
            auto sweep = ramp( random( 8, 32 ), random( .05 ), 1 );
            return settle * settle_drift * sweep * attack;
        };

        map["kick"] = [this]() { return sine( create( "kick_envelope" ) * decay( 2 ) ); };

        map["4floor_kick"] = [this] {
            double bpm = 139;
            return step_sequence( 139, { create( "kick" ) } );
        };

        map["alternating_4floor_kick"] = [this] {
            double bpm = 139;
            return step_sequence( 139, { create( "kick" ), create( "kick" ) } );
        };
    }

    std::shared_ptr<NaiveInstruments::Signal<double>> create( std::string shorthand )
    {
        if ( map.contains( shorthand ) )
            return map[shorthand]();
        else
        {
            std::cerr << "Unexpected shorthand: " << shorthand << "\n";
            throw 1;
        }
    }

    std::shared_ptr<NaiveInstruments::Signal<double>> create( const std::vector<std::string>& shorthandOptions )
    {
        return create( random( shorthandOptions ) );
    }

    std::shared_ptr<NaiveInstruments::Signal<double>> operator()( std::string shorthand )
    {
        return create( shorthand );
    }

    std::vector<std::string> availableCommands()
    {
        std::vector<std::string> list;
        for ( auto i = map.begin(); i != map.end(); ++i )
            list.push_back( i->first );
        return list;
    }
};

} // namespace NaiveInstruments
