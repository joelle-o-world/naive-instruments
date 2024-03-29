#pragma once

#include "../core.h"

class Resetter : public FrameStream<double>
{
public:
    FrameStreamConsumer<double> input{ this, "input" };
    FrameStreamConsumer<bool>   trigger{ this, "trigger/bool" };

    static std::shared_ptr<Resetter> create()
    {
        return std::make_shared<Resetter>();
    }

protected:
    void action() override
    {
        if ( trigger.readFrame() )
            input.reset();
        output[0] = input.readFrame();
    }
};
