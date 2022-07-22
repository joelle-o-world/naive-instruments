#pragma once

#include "../core.h"
#include "WAV_HEADER.h"
#include <cstdint>
#include <fstream>
#include <iostream>

class WavWriter {

  std::ofstream out;
  int numberOfFrames;

public:
  WavWriter(const std::string &outfile, int numberOfFrames)
      : numberOfFrames(numberOfFrames), out(outfile, std::ios::binary) {

    writeHeader();
  }

  int dataSize() { return 2 * sizeof(int16_t) * numberOfFrames; }

  void writeHeader() {
    WAV_HEADER header;

    header.Subchunk2Size = dataSize();
    header.ChunkSize = dataSize() + sizeof(header) - 8;
    // -8 because of the "RIFF{uint32_t ChunkSize}" at the beginning

    out.write(reinterpret_cast<const char *>(&header), sizeof(header));
  }

  void write(int left, int right) {
    int16_t l16bit = left;
    int16_t r16bit = right;
    out.write(reinterpret_cast<const char *>(&l16bit), sizeof(l16bit));
    out.write(reinterpret_cast<const char *>(&r16bit), sizeof(r16bit));
  }

  void write(int value) { write(value, value); }
  void write(double value) { write(doubleToInt16_t(value)); }
  void write(double left, double right) {
    write(doubleToInt16_t(left), doubleToInt16_t(right));
  }

  int16_t doubleToInt16_t(double val) {
    // return 0;
    return INT16_MAX * val * .5;
  }

public:
  void operator<<(double signalPoint) { write(signalPoint); }

  static void write(const std::string &outputFilePath, MonoBuffer &buffer) {
    double attenuation = .9;
    int numberOfFrames = buffer.numberOfFrames();
    WavWriter recorder(outputFilePath, numberOfFrames);
    for (int i = 0; i < numberOfFrames; ++i)
      recorder << buffer[i] * attenuation;
  }
};
