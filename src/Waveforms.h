#pragma once

#include "core.h"
#include "file-io/WavReader.h"
#include <math.h>

class Waveforms
{
public:
    static MonoBuffer& square()
    {
        MonoBuffer& buffer = *( new MonoBuffer( 44100 ) );
        for ( int i = 0; i < buffer.numberOfSamples; ++i )
            buffer.data[i] = i * 2 > buffer.numberOfSamples ? -1.0 : 1.0;
        return buffer;
    }

    static MonoBuffer& sine()
    {
        MonoBuffer& buffer = *( new MonoBuffer( 44100 ) );
        for ( int i = 0; i < buffer.numberOfSamples; ++i )
            buffer.data[i] = sin( M_PI * 2 * i / 44100.0 );
        return buffer;
    }

    static MonoBuffer& saw()
    {
        MonoBuffer& buffer = *( new MonoBuffer( 44100 ) );
        for ( int i = 0; i < buffer.numberOfSamples; ++i )
            buffer.data[i] = float( i ) / float( buffer.numberOfSamples ) * 2 - 1;
        return buffer;
    }

    static MonoBuffer& triangle()
    {
        MonoBuffer& buffer = *( new MonoBuffer( 44100 ) );
        for ( int i = 0; i < buffer.numberOfSamples / 4; ++i )
            buffer.data[i] = i * 4.0 / buffer.numberOfSamples;
        for ( int i = buffer.numberOfSamples / 4; i < .75 * buffer.numberOfSamples; ++i )
            buffer.data[i] = 1.0 - ( i - ( buffer.numberOfSamples / 4.0 ) ) / ( buffer.numberOfSamples / 4.0 );
        for ( int i = buffer.numberOfSamples * .75; i < buffer.numberOfSamples; ++i )
            buffer.data[i] = -1 + ( i - .75 * buffer.numberOfSamples ) / ( buffer.numberOfSamples / 4.0 );


        return buffer;
    }

    static MonoBuffer& fromFile( const std::string& filePath )
    {
        return *NaiveInstruments::WavReader::readMonoFile( filePath.c_str() );
    }

    static MonoBuffer& Malcolm01()
    {
        MonoBuffer& buffer = fromFile( "wavetables/MALCOLM_01.wav" );
        assert( buffer.numberOfFrames() == 162 );
        return buffer;
    }

    // TODO: Probably better to return a pointer not a ref
    static MonoBuffer& byName( const std::string& name )
    {
        if ( name == "sin" || name == "sine" )
            return sine();
        else if ( name == "saw" || name == "sawtooth" )
            return saw();
        else if ( name == "tri" || name == "triangle" )
            return triangle();
        else if ( name == "sq" || name == "square" )
            return square();
        else
            try
            {
                return fromFile( "wavetables/" + name + ".wav" );
            }
            catch ( ... )
            {
                std::cerr << "Couldn't find waveform called '" << name << "'\n";
                throw 1;
            }
    }
};
