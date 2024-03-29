#include "FrameStream.h"

// --------------------------------------------------------------------------------
// class AbstractFrameStreamConsumer
// --------------------------------------------------------------------------------

AbstractFrameStreamConsumer::AbstractFrameStreamConsumer( AbstractFrameStream* owner,
                                                          const std::string&   name,
                                                          bool                 keepSyncedToOwner )
: owner( owner )
, keepSyncedToOwner( keepSyncedToOwner )
, name( name )
{
    owner->inputs.push_back( this );
};

void AbstractFrameStreamConsumer::connect( std::shared_ptr<FrameStream<double>> signal )
{
    std::cerr << "No override for "
                 "AbstractFrameStreamConsumer::connect method\n";
    throw 1;
}

void AbstractFrameStreamConsumer::setConstant( double k )
{
    std::cerr << "No override for "
                 "AbstractFrameStreamConsumer::setConstant(double) method\n";
    throw 1;
}

AbstractFrameStream* untypedConnection = nullptr;

void AbstractFrameStreamConsumer::checkConnection()
{
    if ( untypedConnection == nullptr )
    {
        std::cerr << label() << " has no connection\n";
        throw 1;
    }
}

void AbstractFrameStreamConsumer::reset()
{
    checkConnection();
    untypedConnection->reset();
}

void AbstractFrameStreamConsumer::sync( int clock )
{
    checkConnection();
    untypedConnection->sync( clock );
}

void AbstractFrameStreamConsumer::syncToOwner()
{
    checkConnection();
    if ( keepSyncedToOwner )
        untypedConnection->sync( owner->internalClock );
}

std::string AbstractFrameStreamConsumer::label()
{
    return owner->label() + "." + this->name;
}

short AbstractFrameStreamConsumer::numberOfChannels()
{
    if ( untypedConnection != nullptr )
        return untypedConnection->getNumberOfOutputChannels();
    else
        // TODO: Not sure what to do here. Throw exception? return 0?
        return -1;
}

// --------------------------------------------------------------------------------
// class SignalInput
// --------------------------------------------------------------------------------

#include "../instruments/Constant.h"

template <typename frame>
void FrameStreamConsumer<frame>::setConstant( frame k )
{
    connect( std::make_shared<Constant<frame>>( k ) );
}


template <typename frame>
void FrameStreamConsumer<frame>::connect( std::shared_ptr<FrameStream<frame>> inputSignal )
{
    if ( untypedConnection != nullptr )
    {
        untypedConnection->removeReader( this );
    }
    untypedConnection = inputSignal;
    inputSignal->addReader( this );
    owner->handleConnectionChange();
}

void AVOID_LINKING_ERRORS()
{
    FrameStreamConsumer<bool>   compileItForBools( nullptr, "hello", true );
    FrameStreamConsumer<double> FrameStreamConsumer( nullptr, "hello", true );
}
