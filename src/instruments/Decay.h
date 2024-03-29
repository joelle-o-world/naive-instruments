#include "../core.h"

class Decay : public FrameStream<double>
{
private:
    double phase = 0;

public:
    FrameStreamConsumer<double> duration{
        this,
        "duration/s",
    };

protected:
    void action() override
    {
        if ( phase < 1.0 )
        {
            phase += ( 1.0 / 44100.0 ) / duration.readFrame();
            output[0] = 1.0 - phase;
        }
        else
            output[0] = 0.0;
    }

public:
    void resetState() override
    {
        phase = 0;
    }
};
