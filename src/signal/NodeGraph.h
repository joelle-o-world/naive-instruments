#include "Signal.h"
#include <boost/algorithm/string.hpp>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

/**
 * A class for representing graphs using a plaintext notations. These are intended to be used to show the internal
 * structure of signal processes, but they could be repurposed for other applications.
 */
class NodeGraph
{
protected:
    /** The name of the node */
    const std::string head;

    std::vector<std::shared_ptr<NodeGraph>> inputs;

    NodeGraph* parent;

public:
    NodeGraph( std::string head )
    : head( head )
    {
        parent = nullptr;
    }

    void addInput( std::shared_ptr<NodeGraph> input )
    {
        inputs.push_back( input );
        input->parent = this;
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

    static std::vector<NodeGraph*> backtrace( NodeGraph* node )
    {
        std::vector<NodeGraph*> trace;
        NodeGraph*              current = node;

        do
        {
            trace.push_back( current );
            current = current->parent;
        } while ( current != 0 );

        return trace;
    }


    static NodeGraph* findCommonAncestor( std::shared_ptr<NodeGraph> a, std::shared_ptr<NodeGraph> b )
    {
        auto abt = backtrace( a.get() );
        auto bbt = backtrace( b.get() );

        // Return the first item in `abt` that also exists in `bbt`
        for ( auto itemInA : abt )
            if ( find( bbt.begin(), bbt.end(), itemInA ) != bbt.end() )
                return itemInA;
        // Otherwise
        return nullptr;
    }
    // TODO: add parsing methods
};

class NodeGraphPath
{
    std::vector<std::string> parts;

public:
    std::string toString()
    {
        std::string str = "";
        for ( int i = 0; i < parts.size(); ++i )
        {
            if ( i != 0 )
                str += "/";
            str += parts[i];
        }
        return str;
    }


    // TODO: fromString
};
