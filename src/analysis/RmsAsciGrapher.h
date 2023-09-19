#pragma once

#include "../lib.h"
#include "./RMS.h"
#include <iostream>
#include <string>

class RmsAsciGrapher
{

public:
    static void fromFile( RectangleBuffer<std::string>& canvas,
                          const std::string&            filePath,
                          double                        from = 0,
                          double                        to   = 0 )
    {
        NaiveInstruments::WavReader file( filePath.c_str() );
        int                         width  = canvas.width;
        int                         height = canvas.height;

        if ( to == 0 )
            to = file.duration();
        int fromSample = from * sampleRate;
        int toSample   = to * sampleRate;

        Sampler sampler( NaiveInstruments::WavReader::readMonoFile( filePath.c_str() ) );
        sampler.skipTo( fromSample );

        RMS loudness;
        loudness.windowSize = ( toSample - fromSample ) / width;
        if ( loudness.windowSize < 1 )
        {
            std::cerr << "Too short\n";
            throw 66;
        }

        loudness << sampler;

        canvas.fill( " " );

        RectangularLineTool pen1( canvas );
        RectangularLineTool pen2( canvas );
        pen1.moveTo( 0, canvas.height / 2 );

        for ( int window = 0; window < width; ++window )
        {
            double rms = loudness.nextWindow();

            float y = rms * float( canvas.height );
            float x = float( canvas.width ) * window / width;

            pen1.lineTo( x, canvas.height - y );
        }
    }
};
