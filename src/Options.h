#ifndef OPTIONS_H
#define OPTIONS_H
#include <string>

bool SetOptions(
  // Inputs
  int argc, char * argv[],
  // Outputs
  std::string* config_file,
  std::string* can_port,
  std::string* aux_in_serial,
  std::string* aux_in_baud,
  std::string* out_stream,
  std::string* fwd_stream,
  bool* debug_mode);

#endif // OPTIONS_H