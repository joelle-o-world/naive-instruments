#pragma once
#include "constants.h"
#include <algorithm>
#include <iostream>
#include <math.h>
#include <memory>
#include <sndfile.hh>
#include <vector>

template <typename T>
class ReadableSample
{
    virtual float duration()                                   = 0;
    virtual T     readByFrame( int frame, int channel = 0 )    = 0;
    virtual T     read( float timeInSeconds, int channel = 0 ) = 0;
    virtual T*    frame( int frame )                           = 0;
    virtual int   numberOfFrames()                             = 0;
    virtual int   numberOfChannels()                           = 0;
    virtual int   numberOfSamples()                            = 0;
};

template <typename T>
class Sample : ReadableSample<T>
{

private:
    int _numberOfFrames, _numberOfChannels;

public:
    int numberOfFrames() override
    {
        return _numberOfFrames;
    }
    int numberOfChannels() override
    {
        return _numberOfChannels;
    }
    float sampleRate;

private:
    T* data;

public:
    Sample( int numberOfFrames, int numberOfChannels = 1, float sampleRate = 44100 )
    : _numberOfChannels( numberOfChannels )
    , _numberOfFrames( numberOfFrames )
    , sampleRate( sampleRate )
    {
        data = new T[numberOfSamples()];
    }

    Sample( std::vector<T> _data, int numberOfChannels = 1, float sampleRate = 44100 )
    : Sample( _data.size(), numberOfChannels, sampleRate )
    {
        for ( int i = 0; i < _data.size(); ++i )
            data[i] = _data[i];
    }

    ~Sample()
    {
        delete data;
    }

    float duration() override
    {
        return numberOfFrames() / sampleRate;
    }

private:
    /**
     * Get a reference to a specific cell of the sample data.
     */
    T& cell( int channel, int frame )
    {
        // TODO: add bounds checks
        if ( frame < 0 || frame >= numberOfFrames() )
        {
            std::cerr << "frame out of bounds: " << frame << "\n";
            throw 1; // TODO: Use proper exception
        }
        else if ( channel < 0 || channel >= numberOfChannels() )
        {
            std::cerr << "channel out of bonuds: " << channel << "\n";
            throw 1; // TODO: Use proper exception
        }

        return data[frame * numberOfChannels() + channel];
    }

public:
    T* frame( int f ) override
    {
        return &data[f * numberOfChannels()];
    }

public:
    T* dataptr()
    {
        return data;
    }

    int numberOfSamples() override
    {
        return numberOfChannels() * numberOfFrames();
    }

    // TODO: Define an iterator through a region of neighboring data

public:
    T readByFrame( int frame, int channel = 0 ) override
    {
        return cell( channel, frame );
    }

    T readWithInterpolation( float frame, int channel = 0 )
    {
        // (linear interpolation between samples)
        float progress = fmod( frame, 1 );
        if ( progress == 0 )
            return readByFrame( int( frame ), channel );
        T before = readByFrame( int( frame ), channel );
        T after  = readByFrame( int( frame + 1 ), channel );
        return ( 1.0 - progress ) * before + progress * after;
    }

    T read( float time, int channel = 0 ) override
    {
        int frame = time * sampleRate;
        return readWithInterpolation( frame, channel );
    }

    /**
     * Overwrite a single cell
     */
    void write( T y, int frame, int channel = 0 )
    {
        cell( channel, frame ) = y;
    }

    /**
     * Overwrite a region using another sample
     */
    void write( Sample<T>& y, int offset = 0 )
    {
        // TODO: Support channel offset
        for ( int c = 0; c < y.numberOfChannels(); ++c )
            for ( int readFrame = 0; readFrame < y.numberOfFrames(); ++readFrame )
            {
                int writeFrame = readFrame + offset;
                if ( writeFrame >= 0 && writeFrame < numberOfFrames() )
                    write( y.readByFrame( readFrame, c ), writeFrame, c );
            }
    }

    void overdub( Sample<T>& y, int offset = 0 )
    {
        if ( sampleRate != y.sampleRate )
            throw 1; // TODO: Throw a proper exception
        for ( int c = 0; c < y.numberOfChannels(); ++c )
            for ( int readFrame = 0; readFrame < y.numberOfFrames(); ++readFrame )
            {
                int writeFrame = readFrame + offset;
                if ( writeFrame >= 0 && writeFrame < numberOfFrames() )
                    cell( c, writeFrame ) += y.readByFrame( readFrame, c );
            }
    }

    void operator+=( Sample<T>& other )
    {
        overdub( other );
    }

    void wipe()
    {
        for ( int i = 0; i < numberOfSamples(); ++i )
            data[i] = 0;
    }

    std::shared_ptr<Sample<T>> mix( Sample<T>& other, int offset = 0 )
    {
        if ( sampleRate != other.sampleRate )
            throw 1; // TODO: Throw a proper exception
        auto newSample = std::make_shared<Sample<T>>( std::max( numberOfFrames(), other.numberOfFrames() + offset ),
                                                      std::max( numberOfChannels(), other.numberOfChannels() ),
                                                      sampleRate );

        newSample->wipe();
        newSample->overdub( *this );
        newSample->overdub( other, offset );
        return newSample;
    }

    T peak( int frame0 = 0, int frame1 = 0 )
    {
        if ( frame1 == 0 )
            frame1 = numberOfFrames();

        T max = readByFrame( frame0, 0 );
        for ( int c = 0; c < numberOfChannels(); ++c )
            for ( int frame = frame0; frame < frame1; ++frame )
                max = std::max( max, readByFrame( frame, c ) );
        return max;
    }

    T rms( int frame0 = 0, int frame1 = 0 )
    {
        if ( frame1 == 0 )
            frame1 = numberOfFrames();

        T sum = 0;
        for ( int frame = frame0; frame < frame1; ++frame )
            for ( int channel = 0; channel < numberOfChannels(); ++channel )
            {
                T y = readByFrame( frame, channel );
                sum += y * y;
            }

        int n = ( frame1 - frame0 ) * numberOfChannels();
        return sqrt( sum / n );
    }

    std::shared_ptr<Sample<T>> slice( int frame0 = 0, int frame1 = 0 )
    {
        if ( frame1 == 0 )
            frame1 = numberOfFrames();

        int lengthOfNewSample = frame1 - frame0;

        auto sample = std::make_shared<Sample<T>>( lengthOfNewSample, numberOfChannels() );

        for ( int writeFrame = 0; writeFrame < lengthOfNewSample; ++writeFrame )
        {
            int readFrame = writeFrame + frame0;
            for ( int channel = 0; channel < numberOfChannels(); ++channel )
                sample->write( readByFrame( readFrame, channel ), writeFrame, channel );
        }

        return sample;
    }

    // TODO: writing wav files
    // TODO: Reading wav files

    std::shared_ptr<Sample<T>> selectChannels( std::vector<int>& channelsToSelect )
    {
        auto newSample = std::make_shared<Sample<T>>( numberOfFrames(), channelsToSelect.size() );
        for ( int writeChannel = 0; writeChannel < channelsToSelect.size(); ++writeChannel )
            for ( int frame = 0; frame < numberOfFrames(); ++frame )
                newSample->write( readByFrame( frame, channelsToSelect[writeChannel] ), frame, writeChannel );
        return newSample;
    }

    std::shared_ptr<Sample<T>> stereoFlip()
    {
        std::vector<int> channels = { 1, 0 };
        if ( numberOfChannels() == 2 )
            return selectChannels( channels );
        else
            throw 1; // TODO: Use proper exception
    }

    std::shared_ptr<Sample<T>> selectChannel( int channel )
    {
        std::vector<int> channels = { channel };
        return selectChannels( channels );
    }

    bool operator==( Sample<T>& other )
    {
        if ( numberOfChannels() == other.numberOfChannels() && numberOfFrames() == other.numberOfFrames()
             && sampleRate == other.sampleRate )
        {
            for ( int i = 0; i < numberOfSamples(); ++i )
                if ( data[i] != other.data[i] )
                    return false;
            // otherwise
            return true;
        }
        else
            return false;
    }

    std::shared_ptr<Sample<T>> loop( float howManyTimes )
    {
        auto newSample = std::make_shared<Sample<T>>( howManyTimes * numberOfFrames(), numberOfChannels(), sampleRate );
        for ( int i = 0; i < newSample->numberOfSamples(); ++i )
            newSample->data[i] = data[i % numberOfSamples()];

        return newSample;
    }

    std::shared_ptr<Sample<T>> loopToDuration( float targetDuration )
    {
        return loop( targetDuration / duration() );
    }

    std::shared_ptr<Sample<T>> loopToFrames( float howManyFrames )
    {
        return loop( howManyFrames / numberOfFrames() );
    }

    template <typename U>
    std::shared_ptr<Sample<U>> convertTo()
    {
        auto newSample = std::make_shared<Sample<U>>( numberOfFrames(), numberOfChannels(), sampleRate );
        for ( int frame = 0; frame < numberOfFrames(); ++frame )
            for ( int channel = 0; channel < numberOfChannels(); ++channel )
                newSample->write( ( readByFrame( frame, channel ) ), frame, channel );

        return newSample;
    }

    static std::shared_ptr<Sample<T>> concat( Sample<T>& a, Sample<T>& b )
    {
        int                        numberOfChannels = std::max( a.numberOfChannels(), b.numberOfChannels() );
        int                        numberOfFrames   = a.numberOfFrames() + b.numberOfFrames();
        std::shared_ptr<Sample<T>> sample           = std::make_shared<Sample<T>>( numberOfFrames, numberOfChannels );

        // TODO: add configurable crossfades

        sample->write( a, 0 );
        sample->write( b, a.numberOfFrames() );
        return sample;
    }

    std::shared_ptr<Sample<T>> operator<<( Sample<T>& other )
    {
        return concat( *this, other );
    }

    std::shared_ptr<Sample<T>> operator+( Sample<T>& other )
    {
        return mix( other );
    }

    static std::shared_ptr<Sample<T>> readFile( const std::string& filename )
    {

        SndfileHandle file;
        file = SndfileHandle( filename );

        // Create the container
        std::shared_ptr<Sample<T>> sample =
            std::make_shared<Sample<T>>( file.frames(), file.channels(), file.samplerate() );

        double buffer[file.frames()];

        // TODO: Use bigger chunks
        for ( int frame = 0; frame < file.frames(); ++frame )
        {
            int numberRead = file.readf( buffer, 1 );
            for ( int channel = 0; channel < file.channels(); ++channel )
            {
                sample->write( buffer[channel], frame, channel );
            }
        }

        return sample;
    }

    void writeFile( const std::string& filename )
    {
        SndfileHandle file( filename, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_FLOAT, numberOfChannels(), sampleRate );
        file.write( dataptr(), numberOfSamples() );
    }
};
