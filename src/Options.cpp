#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include "Options.h"

namespace po = boost::program_options;
using namespace std;

const string default_config_file = "/etc/n2kconvert.conf";
const string default_can_port = "can0";
const string default_aux_in_serial = "";
const string default_aux_in_baud = "";
const string default_out_stream = "/dev/stdout";
const double default_depth_offset_ft = 0.0;
const string debug_stream = "/dev/stdout";

bool SetOptions(int argc, char* argv[],
  string* config_file,
  string* can_port,
  string* aux_in_serial,
  string* aux_in_baud,
  string* out_stream,
  string* fwd_stream,
  double* depth_offset_ft,
  bool* debug_mode
  ) {
  *debug_mode = false;
  // Supported command line or config file options.
  po::options_description options_generic("Config or command line options");
  options_generic.add_options()
    ("canport,c", po::value<string>(can_port)->default_value(default_can_port),
      "CAN port to read")
    ("auxin,a", po::value<string>(aux_in_serial)->default_value(default_aux_in_serial),
      "aux serial input of NMEA0183 to overwrite or enhance NMEA2000")
    ("auxinbaud,b", po::value<string>(aux_in_baud)->default_value(default_aux_in_baud),
      "aux serial input baud rate")
    ("output,o", po::value<string>(out_stream)->default_value(default_out_stream),
      "output file/FIFO to send NMEA0183 sentences")
    ("forward", po::value<string>(fwd_stream),
      "output file/FIFO to forward NMEA2000 data")
    ("depth,d", po::value<double>(depth_offset_ft)->default_value(default_depth_offset_ft),
      "depth offset (ft) to apply to transducer (DPT message)")
  ;
  // Supported command line only options
  po::options_description options_cmdline_only("Command line only options");
  options_cmdline_only.add_options()
    ("help", "produce help message")
    ("config,f", po::value<string>(config_file)->default_value(default_config_file), 
      "configuration file name.")
    ("debug,d", "debug mode (send all data to stdout)")
  ;
  // Create list of all options for help
  po::options_description options_all("All options");
  options_all.add(options_generic).add(options_cmdline_only);

  // Parse command line vars
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, options_all), vm);
  po::notify(vm);

  // Display help
  if (vm.count("help")) {
    cout << "n2kconvert - a daemon for converting NMEA2000 CAN data to NMEA0183 sentences.\n";
    cout << options_all <<  "\n";
    exit(0);
  }

  // Open config file
  if (vm.count("config"))
    cout << "Using config file: " << *config_file << "\n";
  ifstream config_fstream(config_file->c_str(), ifstream::in);
  if (!config_fstream) {
    cerr << "Cannot open config file: " << *config_file << "\n";
  } else {
    // Parse config file
    po::store(po::parse_config_file(config_fstream, options_generic), vm);
    po::notify(vm);
  }

  // Handle debug mode
  if (vm.count("debug")) {
    cout << "Debug mode enabled!\n";
    *debug_mode = true;
    *out_stream = debug_stream;
  }
  
  // Display selected ports and streams
  if (vm.count("canport"))
    cout << "Reading from can port: " << *can_port << "\n";
  if (vm.count("auxin"))
    cout << "Reading auxiliary input serial from: "<< *aux_in_serial
         << " using baud rate: " << *aux_in_baud << "\n";
  if (vm.count("output"))
    cout << "Writing NMEA0183 data to: " << *out_stream << "\n";
  if (!fwd_stream->empty())
    cout << "Forwarding NMEA2000 data to: " << *fwd_stream << "\n";
  if (vm.count("depth"))
    cout << "Depth offset set to: " << *depth_offset_ft << "ft\n";

  return true;
}