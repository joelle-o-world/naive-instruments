#pragma once

#include "../lib.h"

class Pauser : public Signal<double> {
  void syncInputs() override {
    // Does nothing
  }

public:
  SignalInput<double> input{this, "input"};

private:
  bool paused = false;
  int t = 0;

public:
  void pause() { paused = true; }
  void resume() { paused = false; }
  void toggle() { paused = !paused; }

  void action() override {
    if (paused)
      out(0);
    else {
      input.sync(t++);
      out(input());
    }
  }
};