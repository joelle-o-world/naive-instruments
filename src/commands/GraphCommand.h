#include "../lib.h"
#include <iostream>
#include <string>

class GraphCommand : public CommandLineApp
{
    using CommandLineApp::CommandLineApp;

public:
    void run() override
    {
        if ( args.boolean( "i" ) )
            interactiveMode();
        else
            stdoutMode();
    }

    void interactiveMode()
    {
        setlocale( LC_ALL, "" );
        initscr();
        mouseinterval( 0 );
        mousemask( BUTTON1_CLICKED | BUTTON4_PRESSED | BUTTON2_PRESSED, NULL );
        MonoBuffer& sample = *NaiveInstruments::WavReader::readMonoFile( args[0].c_str() );

        double from = 0;
        double to   = sample.duration();

        bool             scrub = args.boolean( "scrub" );
        BufferedPlayback playback;
        playback.start( true );

        NaiveInstruments::WavReader file( args[0].c_str() );
        while ( true )
        {
            int c = getch();

            double increment = ( to - from ) / 10;

            switch ( c )
            {
                case KEY_MOUSE:
                    throw 69;
                    break;
                case 'h':
                    if ( from > increment )
                    {
                        from -= increment;
                        to -= increment;
                    }
                    break;
                case 'l':
                    if ( to + increment < sample.duration() )
                    {
                        from += increment;
                        to += increment;
                    }
                    break;
                case '+':
                    to -= increment;
                    from += increment;
                    break;
                case '-':
                    if ( to + increment < sample.duration() && from > increment )
                    {
                        to += increment;
                        from -= increment;
                    }
                    break;
                case '=':
                    from = 0;
                    to   = file.duration();
            }

            clear();
            move( 0, 0 );

            RectangleBuffer<std::string> canvas( 120, 30 );
            RmsAsciGrapher::fromFile( canvas, args[0], from, to );

            move( 3, 0 );
            // printw(cppstr.c_str());
            for ( int y = 0; y < canvas.height; ++y )
                for ( int x = 0; x < canvas.width; ++x )
                {
                    move( y, x );
                    printw( canvas.cell( x, y ).c_str() );
                }

            refresh();

            if ( scrub )
            {
                // TODO: Fix this memory leak!
                auto toPlay = sample.slice( float( from ), float( to ) );
                playback.setSignal( toPlay );
            }
        }
        endwin();
    }

    void stdoutMode()
    {
        for ( int i = 0; i < args.numberOfPositionalArgs(); ++i )
        {
            auto inputFile = args[i];

            RmsAsciGrapher grapher;

            int width  = args.integer( "width", 80 );
            int height = args.integer( "height", 9 );

            RectangleBuffer<std::string> canvas( width, height );

            grapher.fromFile( canvas, inputFile );

            canvas.stream( std::cout );
        }
    }
};
