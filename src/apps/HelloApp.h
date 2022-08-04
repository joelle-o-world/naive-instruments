#include "../cli/Command.h"

class HelloApp : public Command {

  using Command::Command;

  void describeOptions() override {
    // none yet
    options.add_options()("name", po::value<std::string>(),
                          "Tell us your name!");
  }

  void action() override {
    if (args.count("name")) {
      auto name = args["name"].as<std::string>();
      std::cout << "Hello " << name << "\n";
    } else
      std::cout << "Hello World!\n";
  }
};
