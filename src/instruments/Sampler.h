#pragma once

#include "../core.h"
#include "../file-io/WavReader.h"
#include <iostream>

class Sampler : public NaiveInstrument<double> {
private:
  MonoBuffer *buffer;
  int playhead = 0;

public:
  Sampler(const std::string &filePath) {
    buffer = WavReader::readMonoFile(filePath);
  }

  ~Sampler() { delete buffer; }

public:
  double tick() override {
    if (playhead < buffer->numberOfFrames())
      return (*buffer)[++playhead];
    else
      return 0;
  }

  void reset() override { playhead = 0; }

public:
  int numberOfFrames() { return buffer->numberOfFrames(); }
};
