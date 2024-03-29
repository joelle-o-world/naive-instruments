#pragma once
#include "../core.h"
#include "../parsing/Parse.h"
#include "SignalString.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

/**
 * Contianer class so that command line arguments (in raw C form) can be
 * passed around with less code.
 */
class MainArgs
{
public:
    int    argc;
    char** argv;
    MainArgs( int argc, char** argv )
    : argc( argc )
    , argv( argv )
    {
    }
};

const int MISSING_ARGUMENT = 98;

class KeyValue
{
public:
    const std::string& key;
    const std::string& value;

    KeyValue( const std::string& key, const std::string& value )
    : key( key )
    , value( value )
    {
    }
};

class CommandLineArguments
{
    // TODO: Split into two classes, one for core funcitonality and an extension
    // for convenient parser methods
private:
    int    argc;
    char** argv;

private:
    static bool isFlag( char* str )
    {
        if ( str[0] == '-' )
            return true;
        else
            return false;
    }

    static bool isCharFlag( char* str )
    {
        return str[0] == '-' && str[1] != '-' && str[1] != '\0';
    }
    static bool isWordFlag( char* str )
    {
        return str[0] == '-' && str[1] == '-' && str[2] != '\0';
    };

    static int indexOf( char* str, char search )
    {
        char* c = str;
        while ( *c != '\0' )
        {
            if ( *c == search )
                return true;
            ++c;
        }
        // otherwise
        return false;
    }

    std::vector<std::string>           positionalArguments;
    std::map<std::string, std::string> namedArgs;

    int  i = 0;
    bool shift()
    {
        return shiftNamedArg() || shiftPositionalArgument();
    }

    bool shiftPositionalArgument()
    {
        if ( i < argc )
        {
            positionalArguments.push_back( argv[i] );
            i++;
            return true;
        }
        else
            return false;
    }

    bool shiftNamedArg()
    {
        if ( isFlag( argv[i] ) )
            return shiftWordFlag() || shiftCharFlag();
        else
            return false;
    }

    bool shiftWordFlag()
    {
        if ( isWordFlag( argv[i] ) )
        {
            const std::string next            = argv[i];
            const int         assignmentIndex = next.find( "=" );
            if ( assignmentIndex != std::string::npos )
            {
                auto key       = next.substr( 0, assignmentIndex );
                auto value     = next.substr( assignmentIndex + 1 );
                namedArgs[key] = value;
                ++i;
                return true;
            }
            else if ( i + 1 < argc && !isFlag( argv[i + 1] ) )
            {
                std::string key   = next.substr( 2 );
                std::string value = argv[i + 1];
                namedArgs[key]    = value;
                i += 2;
                return true;
            }
            else
            {
                auto key       = next.substr( 2 );
                namedArgs[key] = "";
                ++i;
                return true;
            }
        }
        else
            return false;
    }

    bool shiftCharFlag()
    {
        if ( isCharFlag( argv[i] ) )
        {
            const std::string next = argv[i];
            std::string       key  = next.substr( 1, 1 );

            if ( next.size() > 2 )
            {
                auto value     = next.substr( 2 );
                namedArgs[key] = value;
                ++i;
                return true;
            }
            else if ( i + 1 < argc && !isFlag( argv[i + 1] ) )
            {
                namedArgs[key] = argv[i + 1];
                i += 2;
                return true;
            }
            else
            {
                namedArgs[key] = "";
                ++i;
                return true;
            }
        }
        else
            return false;
    }

    void parseArgs()
    {
        i = 1;
        while ( i < argc && shift() )
            continue;
    }

public:
    CommandLineArguments( int argc, char** argv )
    : argc( argc )
    , argv( argv )
    {
        parseArgs();
    }
    CommandLineArguments( MainArgs& args )
    : CommandLineArguments( args.argc, args.argv )
    {
    }

    const std::string operator[]( int index )
    {
        if ( index < positionalArguments.size() )
            return positionalArguments[index];
        else
        {
            std::cerr << "Missing positional argument at index " << index << "\n";
            throw MISSING_ARGUMENT;
        }
    }
    const std::string operator[]( const std::string& key )
    {
        return namedArgs[key];
    }

    bool exists( const std::string& key )
    {
        return namedArgs.contains( key );
    }
    bool exists( int index )
    {
        return index < positionalArguments.size();
    }

    const int integer( const std::string& key, int fallback )
    {
        if ( exists( key ) )
        {
            auto str = require( key );
            int  val;
            try
            {
                val = std::stoi( str );
            }
            catch ( int err )
            {
                std::cerr << "Argument \"" << key << "\" must be an integer\n";
                throw MISSING_ARGUMENT;
            }
            return val;
        }
        else
            return fallback;
    }

    /**
     * Get floating point input from a named command line argument
     */
    const float number( const std::string& key, float fallback )
    {
        if ( exists( key ) )
        {
            return number( key );
        }
        else
            return fallback;
    }

    const float number( const std::string& key )
    {
        auto  str = require( key );
        float val;
        try
        {
            val = *Parse::number( str );
        }
        catch ( int err )
        {
            std::cerr << "Argument \"" << key << "\" must be an number\n";
            throw MISSING_ARGUMENT;
        }
        return val;
    }

    bool boolean( const std::string& key )
    {
        return exists( key );
    }
    bool flag( const std::string& key )
    {
        return boolean( key );
    }

    const std::string string( const std::string& key, const std::string& fallback )
    {
        if ( exists( key ) )
            return namedArgs[key];
        else
            return fallback;
    }
    const std::string string( int positionalArgIndex, const std::string& fallback )
    {
        if ( positionalArgIndex < numberOfPositionalArgs() )
            return positionalArguments[positionalArgIndex];
        else
            return fallback;
    }

    const std::string require( const std::string& key )
    {
        if ( namedArgs.contains( key ) )
            return namedArgs[key];
        else
        {
            std::cerr << "Missing argument \"" << key << "\"\n";
            throw MISSING_ARGUMENT;
        }
    }

    const int requireInt( const std::string& key )
    {
        try
        {
            auto str = require( key );
            return std::stoi( str );
        }
        catch ( int err )
        {
            std::cerr << "Argument \"" << key << "\" must be an integer\n";
            throw MISSING_ARGUMENT;
        }
    }

    const int numberOfPositionalArgs()
    {
        return positionalArguments.size();
    }

    friend std::ostream& operator<<( std::ostream& os, const CommandLineArguments& args );

    std::shared_ptr<FrameStream<double>> signal( const std::string& key, const std::string& fallback = "0" )
    {
        return SignalString::parse( string( key, fallback ) );
    }
    std::shared_ptr<FrameStream<double>> signal( int key, const std::string& fallback = "0" )
    {
        return SignalString::parse( string( key, fallback ) );
    }

    MainArgs subCommandArguments()
    {
        return { argc - 1, argv + 1 };
    }
};

std::ostream& operator<<( std::ostream& os, const CommandLineArguments& args );
