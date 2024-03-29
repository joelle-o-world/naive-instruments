#pragma once

#include "../core.h"
#include <iostream>
#include <string>
#include <vector>

class BreakpointEnvelope : public FrameStream<double>
{
    void action() override
    {
        progress += progressPerSample;
        if ( progress >= 1 )
            advanceToNextSection();
        output[0] = progress * endValue + ( 1.0 - progress ) * startValue;
    }

public:
    std::string label() override
    {
        return "BreakpointEnvelope";
    }

    BreakpointEnvelope()
    {
        progress               = 1;
        nowPlayingSectionIndex = -1;
    }

    class Section
    {

    public:
        double startValue;
        double endValue;
        double duration;
        double progressPerSample()
        {
            return 1.0 / ( sampleRate * duration );
        }

        double change() const
        {
            return endValue - startValue;
        }
        double gradient() const
        {
            return endValue - startValue / duration;
        }
        bool isHold() const
        {
            return startValue == endValue;
        }

        std::ostream& describe( std::ostream&        out,
                                const Section* const previous = nullptr,
                                const Section* const next     = nullptr ) const
        {

            if ( !previous || previous->duration != duration )
                out << duration << "s:";
            if ( !previous || previous->endValue != startValue || previous->change() != change() )
                out << startValue;
            out << ( isHold() ? "_" : "~" );
            if ( !next || next->startValue != endValue )
                out << endValue << " ";
            return out;
        }
    };

public:
    std::vector<Section> sections;
    void                 addSection( double startValue, double endValue, double duration )
    {
        sections.push_back( { startValue, endValue, duration } );
    }
    void addSection( double endValue, double duration )
    {
        startValue = lastEndValue();
        addSection( startValue, endValue, duration );
    };
    void addHold( double duration )
    {
        auto y = lastEndValue();
        addSection( y, y, duration );
    };

    void closeTheLoop( double duration )
    {
        addSection( lastEndValue(), firstStartValue(), duration );
    }

    double duration()
    {
        double duration = 0;
        for ( int i = 0; i < sections.size(); ++i )
            duration += sections[i].duration;
        return duration;
    }

protected:
    double lastEndValue()
    {
        if ( sections.size() > 0 )
            return sections[sections.size() - 1].endValue;
        else
            return restValue;
    }
    double firstStartValue()
    {
        return ( sections.size() > 0 ) ? sections[0].startValue : restValue;
    }

private:
    double startValue, endValue, progressPerSample, progress;
    void   playSection( Section& section )
    {
        startValue        = section.startValue;
        endValue          = section.endValue;
        progressPerSample = section.progressPerSample();
        progress          = 0;
    }

    bool looped = false;
    void playSection( int sectionIndex )
    {
        nowPlayingSectionIndex = sectionIndex;
        if ( sectionIndex < sections.size() )
            playSection( sections[sectionIndex] );
        else if ( looped )
            playSection( sectionIndex - sections.size() );
        else
            playRestSection();
    }

public:
    void loop()
    {
        looped = true;
    }

protected:
    double restValue;

private:
    void playRestSection()
    {
        startValue        = lastEndValue();
        endValue          = lastEndValue();
        progress          = 0;
        progressPerSample = 0;
    }

    int  nowPlayingSectionIndex;
    void advanceToNextSection()
    {
        playSection( ++nowPlayingSectionIndex );
    }

protected:
    void resetState() override
    {
        playSection( 0 );
    }
};

std::ostream& operator<<( std::ostream& out, const BreakpointEnvelope& B );
std::ostream& operator<<( std::ostream& out, const BreakpointEnvelope::Section& B );
