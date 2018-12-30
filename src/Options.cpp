#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include "Options.h"

namespace po = boost::program_options;
using namespace std;

const string default_config_file = "/etc/n2kconvert.conf";
const string default_can_port = "can0";
const string default_out_stream = "/dev/stdout";

bool SetOptions(int argc, char* argv[],
  string* config_file, string* can_port, string* out_stream,
  bool* background_mode) {
  // Declare the supported options.
  po::options_description options_generic("Config or cmd line options");
  options_generic.add_options()
    ("canport,c", po::value<string>(can_port)->default_value(default_can_port),
      "can port to read.")
    ("output,o", po::value<string>(out_stream)->default_value(default_out_stream),
      "output stream to send data.")
  ;
  po::options_description options_cmdline_only("Command line only options");
  options_cmdline_only.add_options()
    ("help", "produce help message")
    ("config,f", po::value<string>(config_file)->default_value(default_config_file), 
      "configuration file name.")
    ("background,b",
      "run in background.")
  ;
  po::options_description options_all("All options");
  options_all.add(options_generic).add(options_cmdline_only);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, options_all), vm);
  po::notify(vm);
  if (vm.count("help")) {
    cout << options_all <<  "\n";
    exit(0);
  }
  if (vm.count("config"))
    cout << "Using config file: " << *config_file << "\n";
  ifstream config_fstream(config_file->c_str(), ifstream::in);
  if (!config_fstream) {
    cerr << "Cannot open config file: " << *config_file << "\n";
  } else {
    po::store(po::parse_config_file(config_fstream, options_generic), vm);
    po::notify(vm);
  }
  if (vm.count("canport"))
    cout << "Reading can port: " << *can_port << "\n";
  if (vm.count("output"))
    cout << "Writing data to: " << *out_stream << "\n";
  if (vm.count("background")) {
    *background_mode = true;
    cout << "Running in background mode.\n";
  } else {
    *background_mode = false;
  }
  // Change this to true when ready to start using program for real
  return true;
}