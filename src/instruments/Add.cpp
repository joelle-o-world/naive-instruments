#include "Add.h"

// FIXME: This causes a memory leak!
void operator+=(SignalInput<double> &socket, Signal<double> &additionalSignal) {
  Add *add = new Add();
  if (socket.hasPlug()) {
    add->a << socket.currentConnection();
  } else {
    add->a << socket.currentConstant();
  }
  add->b << additionalSignal;
  socket << add;
}
