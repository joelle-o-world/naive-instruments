#pragma once

#include "../core.h"
#include "../file-io/WavReader.h"
#include <iostream>

class Sampler : public FrameStream<double>
{
private:
    std::shared_ptr<MonoBuffer> buffer;
    int                         playhead = 0;

public:
    /**
     * TODO: Remove and replace with static method.
     */
    /* [[deprecated( "Use Sampler(WavReader::readMonoFile(filepath)) instead" )]] Sampler( const std::string& filePath )
     */
    /* { */
    /*     buffer = WavReader::readMonoFile( filePath ); */
    /* } */

    Sampler( std::shared_ptr<MonoBuffer> sharedBuffer )
    {
        buffer = sharedBuffer;
    }

public:
    void action() override
    {
        if ( playhead < 0 )
        {
            ++playhead;
            output[0] = 0;
        }
        else if ( playhead < buffer->numberOfFrames() )
            output[0] = ( *buffer )[++playhead];
        else
            output[0] = 0;
    }

protected:
    void resetState() override
    {
        playhead = 0;
    }

public:
    void skipTo( int frame )
    {
        playhead = frame;
    }

public:
    int numberOfFrames()
    {
        return buffer->numberOfFrames();
    }
};
