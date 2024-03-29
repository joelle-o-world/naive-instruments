
#include "FrameStreamConsumer.h"
#include <set>

/**
 * This class just exists so that FrameStreamConsumer's don't have to know the
 * frame type of their owners.
 */
class AbstractFrameStream
{
    friend AbstractFrameStreamConsumer;

protected:
    short numberOfOutputChannels = 0;

    short getNumberOfOutputChannels()
    {
        return numberOfOutputChannels;
    }

public:
    int internalClock = 0;

protected:
    std::vector<AbstractFrameStreamConsumer*> inputs;

public:
    virtual std::string label()
    {
        return "Signal";
    }
    virtual std::string summary()
    {
        return label() + "@" + std::to_string( internalClock );
    }

public:
    /**
     * Naive instruments work a bit like clock work. Every frame of the digital
     * signal they "tick". What they actually do when they tick is up to them!
     */
    virtual void action()
    {
        std::cerr << "Oh no, looks like the programmer forgot to implement the action "
                     "method for "
                  << label() << "\n";
        throw 1;
    }

public:
    void advanceToNextFrame()
    {
        ++internalClock;
        syncInputs();
        action();
    }
    void sync( int clock )
    {
        while ( internalClock < clock )
        {
            advanceToNextFrame();
        }
    }

private:
    void syncInputs()
    {
        for ( auto input : inputs )
            input->syncToOwner();
    }

protected:
    /**
     * Reset the internal state of the Signal. Stateless signals will NOT
     * override this method
     */
    virtual void resetState()
    {
    }

public:
    virtual void handleConnectionChange()
    {
    }

public:
    void reset()
    {
        // FIXME: Need a circuit breaker for feedback loops
        for ( auto input : inputs )
            input->reset();
        resetState();
    }

public:
    std::vector<std::shared_ptr<AbstractFrameStream>> inputSignals() const
    {
        std::vector<std::shared_ptr<AbstractFrameStream>> list;
        for ( auto input : inputs )
        {
            input->checkConnection();
            list.push_back( input->untypedConnection );
        }
        return list;
    }

    // TODO: Could probable use multiple inheritance to seperate out the readers section.
private:
    std::set<AbstractFrameStreamConsumer*> readers;

public:
    void addReader( AbstractFrameStreamConsumer* newReader )
    {
        readers.insert( newReader );
    }
    void removeReader( AbstractFrameStreamConsumer* oldReader )
    {
        readers.erase( oldReader );
    }

    /**
     * List the streams that are reading this stream via a readers. (not recursive)
     */
    std::set<AbstractFrameStream*> readingStreams()
    {
        // TODO: Check that std::set has move semantics
        std::set<AbstractFrameStream*> streams;
        for ( auto reader : readers )
        {
            streams.insert( reader->owner );
        }
        return streams;
    }
};
