#include <iostream>

class UntypedSignal;
template <typename SignalFrame> class Signal;

class UntypedSignalInput {
protected:
  UntypedSignal *owner;

public:
  UntypedSignalInput(UntypedSignal *owner) : owner(owner){};

  virtual void connect(Signal<double> *signal) {
    std::cerr << "No override for "
                 "UntypedSignalInput::connect(Signal<double>*) method\n";
    throw 1;
  }

public:
  virtual void setConstant(double k) {
    std::cerr
        << "No override for UntypedSignalInput::setConstant(double) method\n";
    throw 1;
  }

protected:
  UntypedSignal *untypedConnection = nullptr;

public:
  bool hasPlug() { return untypedConnection != nullptr; }
};
