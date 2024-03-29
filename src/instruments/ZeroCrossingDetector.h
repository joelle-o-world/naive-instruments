#pragma once

#include "../core.h"

class ZeroCrossingDetector : public FrameStream<bool>
{
private:
    double previous  = 0;
    double threshold = 0;

public:
    FrameStreamConsumer<double> input{ this, "input/audio" };
    bool                        detectDownwardsZeroCrossings = true;
    bool                        detectUpwardsZeroCrossings   = false;

    void action()
    {
        double next   = input.readFrame();
        bool   result = ( detectDownwardsZeroCrossings && previous > threshold && next < -threshold )
                      || ( detectUpwardsZeroCrossings && previous < -threshold && next > threshold );

        if ( next != 0 )
            previous = next;

        output[0] = result;
    }

    void reset()
    {
        previous = 0;
    }
};
