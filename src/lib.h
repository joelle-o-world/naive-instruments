#pragma once

#include "core/MonoBuffer.h"
#include "core/NaiveInstrument.h"
#include "core/Patch.h"
#include "core/Socket.h"
#include "core/constants.h"
#include "file-io/WavReader.h"
#include "file-io/WavWriter.h"
#include "file-io/record.h"

#include "cli/CommandLineApp.h"
#include "cli/CommandLineArguments.h"
#include "cli/LineTool.h"
#include "cli/NCursesWindow.h"
#include "cli/RectangleBuffer.h"
#include "cli/RectangularLineTool.h"
#include "cli/Ruler.h"
#include "cli/TextTool.h"
#include "cli/WindowDimensions.h"

// Instruments
#include "instruments/AHD.h"
#include "instruments/Add.h"
#include "instruments/BreakpointEnvelope.h"
#include "instruments/BufferWriter.h"
#include "instruments/ButterworthFilter.h"
#include "instruments/FixedPipe.h"
#include "instruments/Floor.h"
#include "instruments/LFO.h"
#include "instruments/Multiply.h"
#include "instruments/Noise.h"
#include "instruments/Osc.h"
#include "instruments/PitchConverter.h"
#include "instruments/Ramp.h"
#include "instruments/Sampler.h"
#include "instruments/UnsignedSaw.h"
#include "instruments/Wavetable.h"
#include "instruments/ZeroCrossingDetector.h"

// Analysis
#include "analysis/BangAwaiter.h"
#include "analysis/RmsAsciGrapher.h"
#include "analysis/WaveformBufferer.h"

// Playback
#include "playback/BufferedPlayback.h"
