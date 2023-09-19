#pragma once

#include "../core.h"
#include "../instruments/Constant.h"
#include "../instruments/ControlString.h"
#include "../instruments/Sampler.h"

#include <iostream>
#include <regex>
#include <string>

class SignalString
{
public:
    static std::shared_ptr<FrameStream<double>> parse( const std::string str )
    {

        std::regex wavFileRegex( ".wav$" );
        if ( std::regex_search( str, wavFileRegex ) )
        {
            std::cerr << "SAMPLE: " << str << "\n";
            return make_shared<Sampler>( NaiveInstruments::WavReader::readMonoFile( str.c_str() ) );
        }

        // TODO: use empty shared_ptr for failure?
        Hopefully<std::shared_ptr<ControlString>> cs = ControlString::parse( str );
        if ( cs.success() )
            return *cs;

        try
        {
            double f = std::stoi( str );
            std::cerr << "Got osc " << f << " from " << str << "\n";
            return std::make_shared<Constant<double>>( f );
        }
        catch ( std::invalid_argument err )
        {
            std::cerr << "Can't parse '" << str << "'\n";
            return std::make_shared<Constant<double>>( 0 );
        }
    }
};

void operator<<( FrameStreamConsumer<double>& input, const std::string& str );
