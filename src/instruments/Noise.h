#include "../core.h"

class Noise : public FrameStream<double> {
  Random random;

public:
  Noise(uint64_t seed = 1) : random(seed) {}

private:
  void action() { writeFrame(random.number(-1, 1)); }
};
