#pragma once

#include "../utility/Breadcrumbs.h"
#include "./Constructors.h"
#include "NodeGraph.h"
#include "Signal.h"
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace NaiveInstruments
{

/// A class for analysing the internal structure of signal.
class SignalStructureAnalysis
{
public:
    static std::string signalGraphStructureString( std::shared_ptr<UnknownOutputSignal> signal )
    {
        NodeGraphExtracter instance( signal );
        auto               nodeGraph = instance.extractNodeGraph();
        return nodeGraph->toString();
    }


    /// Get the name (type) of the head signal process
    static std::string signalProcessName( std::shared_ptr<NaiveInstruments::UnknownOutputSignal> signal )
    {
        auto recognised = standardSignalConstructor.recognise( signal );
        if ( recognised != "?" )
            return recognised;

        else
            return typeid( *signal ).name();
    }


    // TODO: Find unset inputs in a signal
    // TODO: list all processes in a signal

    /**
     * A class for taking a signal and mapping out its internal structure using a node graph.
     */
    class NodeGraphExtracter
    {
        /// These breadcrumb pointers are used to circuit break when analyising
        /// signals that contain feedback
        Breadcrumbs<UnknownOutputSignal> breadcrumbs;

    public:
        std::shared_ptr<UnknownOutputSignal> startingPoint;
        NodeGraphExtracter( std::shared_ptr<UnknownOutputSignal> startingPoint )
        : startingPoint( startingPoint )
        {
        }

        std::shared_ptr<NodeGraph> extractNodeGraph()
        {
            return extractNodeGraph( startingPoint );
        }

        std::map<UnknownOutputSignal*, NodeGraph*> map;

        std::shared_ptr<NodeGraph> extractNodeGraph(
            std::shared_ptr<UnknownOutputSignal> signal,
            std::shared_ptr<NodeGraph>           usingNode = std::make_shared<NodeGraph>() )
        {

            // Check for for feedback
            if ( breadcrumbs.crumb( signal ) )
            {
                auto other      = map[signal.get()];
                usingNode->head = NodeGraph::relativePath( usingNode.get(), other );
                return usingNode;
            }
            else
            {
                map[signal.get()] = usingNode.get();
            }

            // Handle constants as a special case
            if ( isConstant( signal ) )
            {
                usingNode->head = stringifyConstant( signal );
                return usingNode;
            }
            else
            {
                // Default handling
                usingNode->head = signalProcessName( signal );
                for ( auto input : signal->inputs )
                {
                    auto childNode = usingNode->addInput();
                    extractNodeGraph( input->abstract_ptr(), childNode );
                }
                return usingNode;
            }
        }
    };

private:
    /// Check if the signal is a mono-constant (`Constant<double>`)
    static bool isConstant( std::shared_ptr<UnknownOutputSignal> signal )
    {
        return typeid( *signal ).name() == typeid( Constant<double> ).name();
    }

    /// Turn a constant signal into a number string
    static std::string stringifyConstant( std::shared_ptr<UnknownOutputSignal> signal )
    {
        auto              constant = std::static_pointer_cast<Constant<double>>( signal );
        std::stringstream s;
        s << constant->output;
        return s.str();
    }
};


} // namespace NaiveInstruments
