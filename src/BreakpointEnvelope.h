#include "NaiveInstrument.h"
#include "Socket.h"
#include <assert.h>
#include <iostream>
#include <vector>

class BreakpointEnvelope : public NaiveInstrument<double> {
  double tick() {
    progress += progressPerSample;
    if (progress >= 1)
      advanceToNextSection();
    return progress * endValue + (1.0 - progress) * startValue;
  }

public:
  BreakpointEnvelope() {
    progress = 1;
    nowPlayingSectionIndex = -1;
  }

  class Section {

  public:
    double startValue;
    double endValue;
    double duration;
    double progressPerSample() { return 1.0 / (sampleRate * duration); }
  };

public:
  std::vector<Section> sections;
  void addSection(double startValue, double endValue, double duration) {
    sections.push_back({startValue, endValue, duration});
  }
  void addSection(double endValue, double duration) {
    startValue = lastEndValue();
  };
  void addHold(double duration);

  double duration() {
    double duration = 0;
    for (int i = 0; i < sections.size(); ++i)
      duration += sections[i].duration;
    return duration;
  }

private:
  double lastEndValue() {
    if (sections.size() > 0)
      return sections[sections.size() - 1].endValue;
    else
      return restValue;
  }

  double startValue, endValue, progressPerSample, progress;
  void playSection(Section &section) {
    startValue = section.startValue;
    endValue = section.endValue;
    progressPerSample = section.progressPerSample();
    progress = 0;
  }

  bool looped = false;
  void playSection(int sectionIndex) {
    nowPlayingSectionIndex = sectionIndex;
    if (sectionIndex < sections.size())
      playSection(sections[sectionIndex]);
    else if (looped)
      playSection(sectionIndex - sections.size());
    else
      playRestSection();
  }

  double restValue;
  void playRestSection() {
    startValue = 0;
    endValue = 0;
    progress = 0;
    progressPerSample = 0;
  }

  int nowPlayingSectionIndex;
  void advanceToNextSection() { playSection(++nowPlayingSectionIndex); }
};
