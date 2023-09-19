#pragma once

#include <iostream>
#include <memory>
#include <sndfile.h>
#include <string>

#include "../core/MonoBuffer.h"
#include "../signal/Signal.h"


namespace NaiveInstruments
{

class WavReader
{

    const char* filepath;
    SNDFILE*    file;
    SF_INFO     info;

    int framesRead;

public:
    /// Read the given file path as a wav file
    WavReader( const char* filepath )
    : filepath( filepath )
    {
        file = sf_open( filepath, SFM_READ, &info );
        checkFile();
    }

    WavReader( FILE* fileStream )
    {
        int fileDescriptor = fileno( fileStream );
        file               = sf_open_fd( fileDescriptor, SFM_READ, &info, 0 );
        checkFile();
    }

private:
    void checkFile()
    {
        if ( !file )
        {
            std::cerr << "Something went wrong opening file " << filepath << "\n";
            // TODO: Use proper exceptions
            throw 1;
        }
    }

    // TODO: Constructor for STDIN


    // TODO: Use some kind of buffering mechanism. Reading 1 frame at a time is inefficient
    /// return 1 frame of the signal as a stereo frame
    StereoFrame readNextFrame()
    {
        if ( numberOfChannels() == 1 )
        {
            double data[1];
            sf_read_double( file, data, 1 );
            ++framesRead;
            return { data[0], data[0] };
        }
        else if ( numberOfChannels() == 2 )
        {
            double data[2];
            sf_read_double( file, data, 2 );
            ++framesRead;
            return { data[0], data[1] };
        }
        else
        {
            std::cerr << "Problem reading a data from " << filepath << "\n";
            // TODO: Use proper exceptions
            throw 1;
        }
    }


public:
    /// Get the sample rate of the audio file
    int sampleRate()
    {
        return info.samplerate;
    }

    /// Count the frames in the audio file
    int numberOfFrames()
    {
        return info.frames;
    }

    int numberOfChannels()
    {
        return info.channels;
    }

    int numberOfFramesRemaining()
    {
        return numberOfFrames() - framesRead;
    }

    /// Get the duration of the audio file in seconds
    float duration()
    {
        return numberOfFrames() / float( sampleRate() );
    }

    /// Read an entire file into memory
    static std::shared_ptr<MonoBuffer> readMonoFile( const char* filepath )
    {
        return WavReader( filepath ).readMonoBuffer();
    }


    // TODO: static method to read stdin into memory as MonoBuffer ptr
    static std::shared_ptr<MonoBuffer> readStdin()
    {
        return WavReader( stdin ).readMonoBuffer();
    }

private:
    std::shared_ptr<MonoBuffer> readMonoBuffer()
    {
        auto buffer = std::make_shared<MonoBuffer>( numberOfFramesRemaining() );
        for ( int i = 0; i < numberOfFrames(); ++i )
            ( *buffer )[i] = readNextFrame().left;
        return buffer;
    }
};

} // namespace NaiveInstruments
