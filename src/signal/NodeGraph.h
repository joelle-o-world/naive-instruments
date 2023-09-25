#include "Signal.h"
#include <boost/algorithm/string.hpp>
#include <memory>
#include <string>
#include <vector>

/**
 * A class for representing graphs using a plaintext notations. These are intended to be used to show the internal
 * structure of signal processes, but they could be repurposed for other applications.
 */
class NodeGraph
{
    /** The name of the node */
public:
    std::string head;

protected:
    std::vector<std::shared_ptr<NodeGraph>> inputs;

    NodeGraph* parent = nullptr;

public:
    NodeGraph( std::string head = "unnamed node" )
    : head( head )
    {
    }

    std::shared_ptr<NodeGraph> addInput( std::shared_ptr<NodeGraph> input = std::make_shared<NodeGraph>() )
    {
        inputs.push_back( input );
        input->parent = this;
        return input;
    }


    std::string toString()
    {
        std::string shorthand = toInlineString();
        // TODO: Max width should be configurable
        if ( shorthand.size() < 80 )
            return shorthand;
        else
            return toMultilineString();
    }

private:
    std::string toInlineString()
    {
        std::string str = head;
        if ( inputs.size() > 0 )
        {
            str += "(";
            // TODO: Could refactor to use array methods
            for ( int i = 0; i < inputs.size(); ++i )
            {
                if ( i != 0 )
                    str += ", ";
                str += inputs[i]->toInlineString();
            }
            str += ")";
        }
        return str;
    }

    std::string toMultilineString()
    {
        std::string str = head;
        if ( inputs.size() > 0 )
        {
            str += ":";
            for ( auto input : inputs )
                str += "\n" + indentInPlace( input->toString() );
        }
        return str;
    }

    static std::string indentInPlace( std::string str )
    {
        std::string prefix = "  ";
        std::string spaces = "  ";
        boost::replace_all( str, "\n", "\n" + spaces );
        return prefix + str;
    }


    // TODO: Move all the path stuff into a separate class
public:
    std::vector<NodeGraph*> backtrace()
    {
        std::vector<NodeGraph*> trace;
        NodeGraph*              ptr = this;
        while ( ptr )
        {
            trace.push_back( ptr );
            ptr = ptr->parent;
        }
        return trace;
    }

    std::string absolutePath()
    {
        std::stringstream str;
        str << "~";
        auto bt = backtrace();
        str << pathifyBacktrace( bt );
        return str.str();
    }

    static std::string pathifyBacktrace( std::vector<NodeGraph*> bt )
    {
        std::stringstream str;
        for ( int i = bt.size() - 1; i > 0; --i )
        {
            auto P = bt[i], C = bt[i - 1];
            int  j = 0;
            while ( j < P->inputs.size() && P->inputs[j].get() != C )
                ++j;
            str << "/" << j;
        }

        return str.str();
    }

    static std::string relativePath( NodeGraph* from, NodeGraph* to )
    {
        std::vector<NodeGraph*> fromBt = from->backtrace(), toBt = to->backtrace();

        if ( fromBt[fromBt.size() - 1] != toBt[toBt.size() - 1] )
            // TODO: Use proper exceptions
            throw 1;

        // Remove the shared root of the backtraces
        while ( fromBt[fromBt.size() - 1] == toBt[toBt.size() - 1] )
        {
            fromBt.pop_back();
            toBt.pop_back();
        }

        std::stringstream str;
        for ( int i = 0; i < fromBt.size(); ++i )
        {
            if ( i != 0 )
                str << "/";
            str << "..";
        }

        str << pathifyBacktrace( toBt );

        return str.str();
    }
    // TODO: add parsing methods
};
