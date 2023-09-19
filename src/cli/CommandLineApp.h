#pragma once

#include "../core.h"
#include "../file-io/WavWriter.h"
#include "../file-io/piping.h"
#include "../file-io/record.h"
#include "../playback/BufferedPlayback.h"
#include "CommandLineArguments.h"
#include <iostream>
#include <memory>

class [[deprecated( "Use Command or sub-class (probably AudioCommand) instead" )]] CommandLineApp
{
public:
    CommandLineArguments args;

public:
    CommandLineApp( int argc, char** argv )
    : args( argc, argv )
    {
    }
    CommandLineApp( MainArgs & args )
    : args( args )
    {
    }

    void output( std::shared_ptr<MonoBuffer> buffer )
    {
        if ( args.boolean( "normalise" ) )
            buffer->normalise();

        if ( stdoutIsAPipe() )
        {
            WavWriter::writeToStdout( *buffer );
        }
        else if ( outputPath().empty() )
        {
            BufferedPlayback::play( buffer );
        }
        else
        {
            WavWriter::write( outputPath().c_str(), *buffer );
        }
    }

    std::shared_ptr<MonoBuffer> mainInputAsBuffer()
    {
        if ( args.exists( "input" ) )
        {
            const auto filename = args["input"];
            return NaiveInstruments::WavReader::readMonoFile( filename.c_str() );
        }
        else if ( stdinIsAPipe() )
        {
            return NaiveInstruments::WavReader::readStdin();
        }
        else
        {
            // TODO: implement some kind of array shifting with positional arguments
            const auto filename = args[0];
            return NaiveInstruments::WavReader::readMonoFile( filename.c_str() );
        }
    }

    float duration()
    {
        const auto durationStr = args.require( "duration" );
        try
        {
            return *Parse::interval( durationStr );
        }
        catch ( ... )
        {
            std::cerr << "Couldn't parse --duration option\n";
            throw 1;
        }
    }

    void output( std::shared_ptr<FrameStream<double>> signal )
    {
        auto& path = outputPath();
        if ( stdoutIsAPipe() )
        {
            recordToStdout( signal, duration() );
        }
        else if ( path.empty() )
            BufferedPlayback::play( signal );
        else
        {
            record( path, signal, duration() );
            std::cout << path << "\n";
        }
    }


    const std::string outputPath()
    {
        return args.string( "o", "" );
    }

    // TODO: getSample() for getting input samples

private:
    virtual void run()
    {
        std::cout << "Not implemented...\n";
    }

public:
    void operator()()
    {
        run();
    }

protected:
    void error( const std::string& errMessage )
    {
        std::cerr << "Error: " << errMessage << "\n";
    }
};
