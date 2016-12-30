#ifndef __OPT_INFO_H
#define __OPT_INFO_H

#include <string>

#define NUM_OPTS 16
#define SHORT_NAME 0
#define LONG_NAME 1

using std::string;

class OptInfo {
public:
  string long_name;     // Long name of this option; used in
			// configuration file and with the -- marker
			// on the command line
  string short_name;    // Short name of this option; used with the -
			// marker on the command line
  int set;              // Flag indicating if this option has already
                        // been set 
  char type;            // type of value: legitimate values are f
			// (flag, i.e., boolean), i (int), s (string)
			// or h (help)
  union {               // pointer to actual value to be set
    int    *flag_val;   // value if type = 'f'
    int    *int_val;    // value if type = 'i'
    string *str_val;    // value if type = 's'
  };    

  OptInfo() {};
};

#endif
  
