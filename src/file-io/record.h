#pragma once

#include "../core/FrameStream.h"
#include "../signal/SignalShorthands.h"
#include "WavWriter.h"
#include <string>

/**
 * Record a signal into a WAV file.
 */
void record( const std::string& outputFile, std::shared_ptr<FrameStream<double>> signal, float duration );
void record( const std::string& outputFile, NaiveInstruments::SignalShorthands::mono signal, float duration );
void record( std::ostream& outputStream, std::shared_ptr<FrameStream<double>> signal, float duration );
void record( std::ostream& outputStream, NaiveInstruments::SignalShorthands::mono, float duration );
