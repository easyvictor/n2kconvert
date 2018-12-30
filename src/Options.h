#ifndef OPTIONS_H
#define OPTIONS_H
#include <string>

bool SetOptions(int argc, char * argv[], // inputs
  std::string* config_file, std::string* can_port, std::string* out_stream); // output

#endif // OPTIONS_H