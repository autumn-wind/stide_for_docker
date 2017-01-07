#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include "config.h"
#include "opt_info.h"

#define LF_LIM 999
#define SEQ_LEN_LIM 199
#define MAX_ELEM_LIM 999
#define MAX_STREAMS_LIM 9999

using std::vector;
using std::cout;
using std::cerr;
using std::endl;

/**********************************************************************
 * Config()                                                           *
 *   Reads in configuration information from configuration file, from *
 *   the command line, and from preset defaults.                      *
 *                                                                    *
 * Input: int argc:      Number of arguments on command line          *
 *        char *argv[]:  Array of strings of actual arguments         *
 *                                                                    *
 * Output: Nothing                                                    *
 *********************************************************************/

Config::Config() 
{
  vector<OptInfo> opt_array(NUM_OPTS);
  InitOptArray(opt_array);

  SetDefaults();

  //ReadCommandLine(argc, argv, opt_array);

  ReadConfigFile(opt_array);

  CheckValues();

  InitOutputFormat();

  OuputConfigInfo(opt_array);

}

/*********************************************************************
 * InitOptArray()                                                    *
 *   Sets the values of opt_array so that opr_array contains all the *
 *   information needed about the parameters being set by the config *
 *   file and the command-line arguments.                            *
 *                                                                   *
 * Input:  vector<OptInfo> &opt_array:  Array of information about   *
 *                                     options for the program       *
 *                                                                   *
 * Output: Nothing                                                   *
 *********************************************************************/

void Config::InitOptArray(vector<OptInfo> &opt_array)
{
//  opt_array.reserve(NUM_OPTS);

  opt_array[0].long_name = "db_name";
  opt_array[0].short_name = "d";
  opt_array[0].set = 0;
  opt_array[0].type = 's';
  opt_array[0].str_val = &db_name;

  opt_array[1].long_name = "seq_len";
  opt_array[1].short_name = "l";
  opt_array[1].set = 0;
  opt_array[1].type = 'i';
  opt_array[1].int_val = &seq_len;

  opt_array[2].long_name = "max_elements";
  opt_array[2].short_name = "me";
  opt_array[2].set = 0;
  opt_array[2].type = 'i';
  opt_array[2].int_val = &max_elements;

  opt_array[3].long_name = "max_streams";
  opt_array[3].short_name = "ms";
  opt_array[3].set = 0;
  opt_array[3].type = 'i';
  opt_array[3].int_val = &max_streams;

  opt_array[4].long_name = "cfg_name";
  opt_array[4].short_name = "c";
  opt_array[4].set = 0;
  opt_array[4].type = 's';
  opt_array[4].str_val = &cfg_name;

  opt_array[5].long_name = "pair_offset";
  opt_array[5].short_name = "p";
  opt_array[5].set = 0;
  opt_array[5].type = 'i';
  opt_array[5].int_val = &pair_offset;

  opt_array[6].long_name = "add_output_format";
  opt_array[6].short_name = "aof";
  opt_array[6].set = 0;
  opt_array[6].type = 's';
  opt_array[6].str_val = &add_output_format;

  opt_array[7].long_name = "compare_output_format";
  opt_array[7].short_name = "cof";
  opt_array[7].set = 0;
  opt_array[7].type = 's';
  opt_array[7].str_val = &compare_output_format;

  opt_array[8].long_name = "add_to_db";
  opt_array[8].short_name = "a";
  opt_array[8].set = 0;
  opt_array[8].type = 'f';
  opt_array[8].int_val = &add_to_db;

  opt_array[9].long_name = "output_graph";
  opt_array[9].short_name = "g";
  opt_array[9].set = 0;
  opt_array[9].type = 'f';
  opt_array[9].int_val = &output_graph;

  opt_array[10].long_name = "compute_hdist";
  opt_array[10].short_name = "hd";
  opt_array[10].set = 0;
  opt_array[10].type = 'f';
  opt_array[10].int_val = &compute_hdist;

  opt_array[11].long_name = "lf_size";
  opt_array[11].short_name = "lf";
  opt_array[11].set = 0;
  opt_array[11].type = 'i';
  opt_array[11].int_val = &lf_size;

  opt_array[12].long_name = "write_db_stats";
  opt_array[12].short_name = "s";
  opt_array[12].set = 0;
  opt_array[12].type = 'f';
  opt_array[12].int_val = &write_db_stats;

  opt_array[13].long_name = "verbose";
  opt_array[13].short_name = "v";
  opt_array[13].set = 0;
  opt_array[13].type = 'f';
  opt_array[13].int_val = &verbose;

  opt_array[14].long_name = "very_verbose";
  opt_array[14].short_name = "V";
  opt_array[14].set = 0;
  opt_array[14].type = 'f';
  opt_array[14].int_val = &very_verbose;

  opt_array[15].long_name = "help";
  opt_array[15].short_name = "h";
  opt_array[15].set = 0;
  opt_array[15].type = 'h';

}

/*********************************************************************
 * SetDefaults()                                                     *
 *   Sets conifiguration variables to their default values           *
 *                                                                   *
 * Input: None                                                       *
 *                                                                   *
 * Output: None                                                      *
 ********************************************************************/

void Config::SetDefaults()
{
  cfg_name = "stide.config";
  db_name = "default.db";
  seq_len =  6;
  max_elements = 500;
  max_streams = 500;
  pair_offset = 0;
  add_output_format = "DB Size: %d\tStream: %s\tPair Number: %p\n";
  compare_output_format = "Pair Number: %p\tStream Number: %s\n";
  lf_size = 1;
  add_to_db = 0;
  output_graph = 0;
  compute_hdist = 0;
  write_db_stats = 0;
  verbose = 0;
  very_verbose = 0;
  num_fvars = 0;
}


/*********************************************************************
 * ReadCommandLine()                                                 *
 *   Parses the command line.  Updates configuration variables.      *
 *                                                                   *
 *  const int argc             Number of arguments                   *
 *  const char *argv[],        Array of arguments                    *
 *  vector<OptInfo> &opt_array  Constant array of information about   *
 *                             the configuration variables           *
 ********************************************************************/

void Config::ReadCommandLine(const int argc, const char *argv[], 
			     vector<OptInfo> &opt_array)
{
  string var_name;     // Name of variable
  string var_val;      // Value of variable
  int name_type;       // LONG_NAME or SHORT_NAME
  int argv_i = 1;      // First index of argv
  int argv_j = 0;      // Second index of argv

  while (argv_i < argc) {
    if (argv[argv_i][argv_j] != '-') {
      cerr<< "ERROR: Switches must be preceeded by a dash: "<<argv[argv_i]
	  << endl << " is illegal" << endl;
      exit(-1);
    }
    argv_j++;
    if (argv[argv_i][argv_j] == '-') { // Long name
      argv_j++;
      name_type = LONG_NAME;
    }
    else {
      name_type = SHORT_NAME;
    }

    // Read name into var_name
    var_name = argv[argv_i]+argv_j;

    // Now we want to read the value, if there is one.  
    argv_j = 0;
    if (++argv_i < argc) {
      if (argv[argv_i][argv_j] != '-') {
	var_val = argv[argv_i];
	argv_i++;
      }
    }

    // assign value to appropriate variable
    AssignValToVar(opt_array, var_val, var_name, name_type);
    // Blank var_name and var_val for next time around
    var_name.resize(0);
    var_val.resize(0);
  }
}

/*********************************************************************
 * AssignValToVar()                                                  *
 *   Figures out which variable to assign a given value to and does  *
 *   so.  Updates opt_array, to say that that particular variable    *
 *   has been set.                                                   *
 *                                                                   *
 * Input: vector<OptInfo> &opt_array  Option Information              *
 *        const string &var_val      Value to be assigned            *
 *        const string &var_name     Name of variable to be updated  *
 *        const int name_type        SHORT_NAME or LONG_NAME         *
 *                                                                   *
 * Output: None                                                      *
 ********************************************************************/

void Config::AssignValToVar(vector<OptInfo> &opt_array, const string
			    &var_val, const string &var_name, const
			    int name_type) 
{
  int opt_i;

  for (opt_i = 0; opt_i < NUM_OPTS; opt_i++) {
    if (((name_type == LONG_NAME) && (opt_array[opt_i].long_name ==
				      var_name)) ||
	((name_type == SHORT_NAME) && (opt_array[opt_i].short_name ==
				       var_name))) { 
      // If we have already set this variable and shouldn't change it,
      // don't 
      if (opt_array[opt_i].set == 1) {
	break;
      }
      switch (opt_array[opt_i].type) {
      case 'f':         // flag
	if ((var_val.length() == 0) || (var_val == "On") || 
	    (var_val == "ON") || (var_val == "on")) { 
	  *(opt_array[opt_i].flag_val) = 1;
	  opt_array[opt_i].set = 1;
	}
	else if ((var_val != "Off") && (var_val != "off") && 
		 (var_val != "OFF")) {
	  cerr << "ERROR: Illegal value for parameter " << var_name
	       << ".  This parameter is a simple flag," << endl
	       << "and may be followed by \"on\", \"off\", or nothing "
               << "(which turns it on).  The current value is " 
	       << var_val << ".  Aborting...";
	  exit(-1);
	}
	break;
      case 'i':
	// If there isn't a value, just use the default
	if (var_val.length() == 0) { 
	  break;
	}
	*(opt_array[opt_i].int_val) = atoi(var_val.c_str());
	opt_array[opt_i].set = 1;
	break;
      case 's':
	// If there is no string given, just use the default
	if (var_val.length() == 0) { 
	  break;
	}
	*(opt_array[opt_i].str_val) = var_val;
	opt_array[opt_i].set = 1;
	break;
      case 'h':
	WriteHelpInfo();
      } // end of switch
      return; // we've found it, so we're done
    } // end of if (opt_array[opt_i]...
  } // end of for (opt_i = 0; ...
}


/*********************************************************************
 *  ReadConfigFile()                                                 *
 *    Parses the configuration file.  Updates configuration          *
 *    variables.                                                     *
 *                                                                   *
 *  Input: vector<OptInfo> &opt_array: Option information             *
 *                                                                   *
 *  Output: None                                                     *
 ********************************************************************/


void Config::ReadConfigFile(vector<OptInfo> &opt_array)
{
  string var_name;
  string var_val;

  // Set up stream for reading configuration
  ifstream cfg_file(cfg_name.c_str());
  string buff;
  int buff_i = 0;  // index for buff
  int opt_i = 0;   // index for opt_array
  int rev_num;     // revision number of configuration file

  if (!cfg_file.is_open()) {
    cerr<<"WARNING: Cannot open configuration file "<<cfg_name
        <<".  I will continue, using the" <<endl
	<<"default values and the command line arguments." << endl
        <<"If that isn't what you wanted, type Ctrl-C now to abort."
	<< endl;
    return;
  }

  // First we need to determine if the configuration file is old-style
  // or new-style, i.e., is there a #ConfigFileRev: in the first
  // line.  We can determine this just be checking the first
  // character. 
  char c = cfg_file.peek();

  // Config file is empty; just return
  if (cfg_file.eof()) {
    return;
  }

  // If old-style
  if (c != '#') {
    cerr << "WARNING: The first line of the configuration file did "
         << "not contain the string" << endl
         << "\"#ConfigFileRev: " << CFREV << "\"." << endl
         << "I will assume that this is an old format configuration "
         << "file." << endl
         <<"If that isn't what you wanted, type Ctrl-C now to abort."
	 << endl << endl;
    ReadOldConfigFile(cfg_file, opt_array);
    return;
  }

  // Look for "#ConfigFileRev:"
  cfg_file >> buff;
  
  if (buff != "#ConfigFileRev:") {
    cerr << "ERROR: I expected the first line of the configuration "
         << "file to either be \"#ConfigFileRev: \" followed by the "
         << "revision number or the beginning of an old-style "
         << "configuration file, which does not have a comment in the "
         << "first line.  I'm confused, so I will abort..." 
	 << endl << endl;
    exit(-1);
  }
    
  cfg_file >> rev_num;

  if (rev_num > CFREV) {
    cerr << "ERROR: This version of STIDE does not know how to deal "
         << "with configuration files" << endl
         << "more modern than revision " << CFREV << ".  Aborting..."
	 <<endl; 
    exit(-1);
  }
  if (rev_num < CFREV) {
    cerr << "ERROR: Configuration files must be revision " << CFREV
	 << "or later, " << "or an old-style" << endl
         << "configuration file without a revision number.  "
         << "Aborting..." << endl; 
    exit(-1);
  }

  // Now we know everything's as we expect, so we'll parse the file

  while (!cfg_file.eof()) {
    // Skip white space at the beginning of the line
    while (isspace(buff[buff_i])) { 
      buff_i++; 
    }

    // If buff is empty, move on to next line
    if (buff.length() <= buff_i) {
      getline(cfg_file, buff);
      buff_i = 0;
      continue;
    }      

    // If we start with a comment, move on to next line
    if (buff[buff_i] == '#') {
      getline(cfg_file, buff);
      buff_i = 0;
      continue;
    }
    // Read in variable name, up to the : 
    int start_place = buff_i; // the beginning place of the name
    while (buff[buff_i] != ':' && (buff_i < buff.length())) { 
      buff_i++;
    }  
    if (buff[buff_i] == buff.length()) {
      cerr << "ERROR: Variable names in the configuration file must "
	   << "be followed by a colon.  The line " << endl
	   << buff << endl << "contains a variable name which is not "
	   << "terminated by a colon.  Aborting..." <<endl;
      exit(-1);
    }

    // This assigns the values in buff between start_place and buff_i
    // to var_name
    var_name.assign(buff, start_place, buff_i - start_place); 

    // Skip colon
    buff_i++;

    // Skip white space
    while (isspace(buff[buff_i])) { buff_i++; }

    start_place = buff_i; // the starting place of the value
    // Find last point in value. If it starts with a quote, it ends
    // with a quote.
    if ((buff[buff_i] == '\"') && (buff_i < buff.length())) {
      while (buff[buff_i] != '\"') {  
	buff_i++;
      }
      // Strip off first "
      start_place++;
    }
    // Otherwise, it ends with a space, a # or the end of the line
    else {
      while ((buff_i < buff.length()) && (!isspace(buff[buff_i])) && 
	     (buff[buff_i] != '#')) {  
	buff_i++;
      }
    }
    var_val.assign(buff, start_place, buff_i - start_place); 

    // Now we want to check to see if the line was continued, in which
    // case we haven't gotten the value of the variable in var_val, so
    // we still need to do that.
    if (buff[buff_i-1] == '\\') {
      getline(cfg_file, buff);
      buff_i = 0;
      while (isspace(buff[buff_i])) { buff_i++; }
      start_place = buff_i;
      // Find last point in value. If it starts with a quote, it ends with a
      // quote.
      if (buff[buff_i] == '\"') {
	buff_i++;
	while ((buff[buff_i] != '\"') && (buff_i < buff.length())) {  
	  buff_i++;
	}
	start_place++; // Strip off first "
      }
      // Otherwise, it ends with a space, a # or the end of the line
      else {
	while ((buff_i < buff.length()) && (!isspace(buff[buff_i])) && 
	       (buff[buff_i] != '#')) {  
	  buff_i++;
	}
      }
      var_val.assign(buff, start_place, buff_i - start_place); 
    }

    // assign value to appropriate variable
    AssignValToVar(opt_array, var_val, var_name, LONG_NAME);
    getline(cfg_file, buff);
    buff_i = 0;
  } //end of while (!cfg_file.eof())...

}


/*********************************************************************
 *  ReadOldConfigFile()                                              *
 *    Reads information from an old-style configuration file.        *
 *    Updates configuration variables.                               *
 *                                                                   *
 *  Input: ifstream &cfg_file   Configuration file (already opened)  *
 *         vector<OptInfo> &opt_array: Option information             *
 *                                                                   *
 *  Output: None                                                     *
 ********************************************************************/

void Config::ReadOldConfigFile(ifstream &cfg_file, 
			       vector<OptInfo> &opt_array)
{

  string buff;
  string var_name;
  string var_val;

  var_name = "max_elements";
  cfg_file>>var_val;
  AssignValToVar(opt_array, var_val, var_name, LONG_NAME);
  getline(cfg_file, buff);

  var_name = "max_streams";
  cfg_file>>var_val;
  AssignValToVar(opt_array, var_val, var_name, LONG_NAME);
  getline(cfg_file, buff);

  // Next line is hash table size, but we are now figuring that out
  // dynamically, so just throw it away.
  getline(cfg_file, buff);

  // Now read in the format string
  getline(cfg_file, var_val);
  // Put the format string in the appropriate place
  if (add_to_db) {
    var_name = "add_output_format";
    AssignValToVar(opt_array, var_val, var_name, LONG_NAME);
  }
  else {
    var_name = "compare_output_format";
    AssignValToVar(opt_array, var_val, var_name, LONG_NAME);
  }
}

/*********************************************************************
 *  CheckValues()                                                    *
 *    Checks configuration values that have been read in to make     *
 *  sure that they are within the limits.  Flags are automatically   *
 *  checked while being read in, the output formats are checked      *
 *  in InitOutputFormat(), and filenames are checked when they are   *
 *  opened, so all that is left is the integer values.               *
 *                                                                   *
 *  Input: None                                                      *
 *                                                                   *
 *  Output: None                                                     *
 ********************************************************************/

void Config::CheckValues()     
{
  if ((lf_size < 1) || (lf_size > LF_LIM)) {
    cerr << "ERROR: lf_size must be between 1 and " << LF_LIM
         << ".  It has been set to " << lf_size << ".  Aborting..." << endl;
    exit(-1);
  }
  if ((seq_len < 1) || (seq_len > SEQ_LEN_LIM)) {
    cerr << "ERROR: seq_len must be between 1 and " << SEQ_LEN_LIM
         << ".  It has been set to " << seq_len << ".  Aborting..." << endl;
    exit(-1);
  }
  if ((max_elements < 1) || (max_elements > MAX_ELEM_LIM)) {
    cerr << "ERROR: max_elements must be between 1 and " << MAX_ELEM_LIM
         << ".  It has been set to " << max_elements    
	 << ".  Aborting..." << endl;
    exit(-1);
  }
  if ((max_streams < 1) || (max_streams > MAX_STREAMS_LIM)) {
    cerr << "ERROR: max_streams must be between 1 and " << MAX_STREAMS_LIM
         << ".  It has been set to " << max_streams     
	 << ".  Aborting..." << endl;
    exit(-1);
  }
}                        



/*********************************************************************
 *  InitOutputFormat()                                               *
 *    Converts the string add_output_format or compare_output_format *
 *  to information filling fmt_str and num_fvars, which is more      *
 *  convenient for output.                                           *
 *                                                                   *
 *  Input: None                                                      *
 *                                                                   *
 *  Output: None                                                     *
 ********************************************************************/

void Config::InitOutputFormat()
{
  // Now we analyze add_output_format or compare_output_format
  int flag = 0;
  int f_i = 0;
  num_fvars = 0;
  string *buff;

  // If we're not in verbose or very_verbose modes, we're never going
  // to use this information, so don't waste our time doing this
  if (!(verbose || very_verbose)) {
    return;
  }
  if (add_to_db) {
    buff = &add_output_format;
  }
  else {
    buff = &compare_output_format;
  }

  for (int i = 0; i <(*buff).length(); i++) {
    switch ((*buff)[i]) {
    case '\\':
      i++;
      switch ((*buff)[i]) {
          case 't': fmt_str[num_fvars][f_i] = '\t'; break;
          case 'n': fmt_str[num_fvars][f_i] = '\n'; break;
	}
	break;
    case '%':
	fmt_str[num_fvars][f_i] = '%';
	flag = 1;
	break;
    default:
      fmt_str[num_fvars][f_i] = (*buff)[i];
      if (flag) {
	switch (fmt_str[num_fvars][f_i]) {
	case 'd':      // database size
	case 'i':      // number of last value of sequence in this
		       // data stream 
	case 'p':      // number of last value of sequence in entire
		       // input
	case 's':      // external stream ID
	case 'a':      // flag for whether this sequence is anomalous 
	case 'c':      // locality frame count of this sequence
	case 'h':      // Hamming distance for this sequence
	  // Record that we must write that val at that position
	  write_val[num_fvars] = fmt_str[num_fvars][f_i]; 
	  fmt_str[num_fvars][f_i] = 'd';
	  fmt_str[num_fvars][f_i + 1] = '\0';
	  num_fvars++;
	  f_i = -1;
	  flag = 0;
	  break;
	default:        // Unknown flag
	  cerr << "ERROR: Illegal control character in output format."
               << "  Type stide -h for help." << endl; 
	} 
      }
    }  // switch ((*buff)[i ...
    f_i++;
  }
  fmt_str[num_fvars][f_i] = '\0';
}



/*********************************************************************
 * OutputConfigInfo()                                                *
 *   Writes information about the final configuration to standard    *
 *   output.  Does so in a format that could be used as a            *
 *   configuration file.  Changes no values anywhere.                *
 *                                                                   *
 * Input: const vector<OptInfo> &opt_array  Option Information        *
 *                                                                   *
 * Output:  None                                                     *
 ********************************************************************/

void Config::OuputConfigInfo(const vector<OptInfo> &opt_array) const
{
  cout<<"This run was configured using configuration file " 
      << cfg_name << " and command" << endl
      << "line arguments.  The configuration values were as "
      << "follows." << endl  
      <<"#ConfigFileRev: " << CFREV << endl;
  for (int i = 0; i < NUM_OPTS; i++) {
    if (opt_array[i].type == 'i') {
      cout << opt_array[i].long_name << ": " << *(opt_array[i].int_val)
	   << endl;
    }
    if ((opt_array[i].type == 's') && 
	((add_to_db &&	(opt_array[i].short_name == "aof")) ||
	 (!add_to_db && (opt_array[i].short_name == "cof")))) {
      cout << opt_array[i].long_name << ": \"" << *(opt_array[i].str_val) 
	   << "\"" << endl;
    }
    if (opt_array[i].type == 'f') {
      if (*(opt_array[i].int_val) == 1) {
	cout << opt_array[i].long_name << ": On" << endl;
      }
      if (*(opt_array[i].int_val) == 0) {
	cout << opt_array[i].long_name << ": Off" << endl;
      }
    }
  }
  cout << endl << endl;

  // Now print header for verbose modes
  if (verbose || very_verbose) {
    cout<<endl<<"Variables in output: "<<endl;
    for (int j = 0; j < num_fvars; j++) {
      switch (write_val[j]) {
        case 's': cout<<"stream #, "; break;
        case 'i': cout<<"index #, "; break;
        case 'h': if (compute_hdist) {cout<<"hamming miss, "; } break;
        case 'c': if (lf_size > 1) {cout<<"lfc, "; } break;
        case 'p': cout<<"pair #, "; break;
        case 'd': cout<<"db size, "; break;
        case 'a': cout<<"is anomalous?, "; break;
      }
    }
    cout<<endl;
  }

}  

/*********************************************************************
 *  WriteHelpInfo()                                                  *
 *    Writes help information to standard output.  Changes no values.*
 *                                                                   *
 *  Input: None                                                      *
 *                                                                   *
 *  Output: None                                                     *
 *                                                                   *
 ********************************************************************/


void Config::WriteHelpInfo() const
{
  cout<<"STIDE accepts calls of the form:"<<endl
      <<" stide -c cfg_name -d db_name -e max_num_elements" 
      <<" -lf lf_size -l seq_len"<<endl<<"    -n max_num_streams"
      <<" -p pair_num_offset -aof add_out_format "
      << endl << "    -cof comp_out_format -a -g -h -m -s -v -V" 
      << endl << endl;  
  cout<<"STIDE expects input to come through standard input in"
      <<" the format of a pair"<<endl
      <<"of integers per line, where the first integer is a"
      <<" stream identifier"<<endl
      <<"and the second is a data element.  Command line"
      <<" arguments override"<<endl
      <<"specifications in the configuration file.  All"
      <<" parameters are optional"<<endl
      <<"and can be specified in any order.  Parameters"
      <<" are always preceded by a"<<endl 
      <<"switch.  The switches are:"<<endl<<endl;
  cout<<"-a                   Add to database; defaults to off"<<endl; 
  cout<<"-c cfg_name          The name of file containing the"
      <<" configuration;"<<endl
      <<"                     defaults to \"stide.config\""<<endl;
  cout<<"-d db_name           The name of the file containing"
      <<" the database;"<<endl
      <<"                     defaults to \"default.db\""<<endl; 
  cout<<"-lf lf_size           The size of the locality frame;"
      <<" defaults to 1"<<endl;
  cout<<"-g                   Write graphing data in dot format to"
      <<" db_name.dot;"<<endl
      <<"                     defaults to off"<<endl;
  cout<<"-h                   Help; displays this information"<<endl;
  cout<<"-l seq_len           Length of sequence; defaults to 6"
      <<endl;
  cout<<"-p pair_offset       Offset for pair number count;"
      <<" defaults to 0"<<endl;
  cout<<"-s                   Display db stats; defaults to off"
      <<endl; 
  cout<<"-v                   Verbose mode on; defaults to off"<<endl;
  cout<<"-V                   Very verbose mode on; defaults to off"<<endl;
  cout<<"-hd                  Compute Hamming distance measures;"
      <<" defaults to off"<<endl;
  cout<<"-me max_elements     Maximum number of different"
      <<" elements"<<endl
      <<"                     in the input stream; defaults to"
      <<" 500" <<endl;
  cout<<"-ms max_num_streams  Maximum number of different"
      <<" streams in input;"<<endl
      <<"                     defaults to 100"<<endl;
  cout<<"-aof add_out_format  Format for output when adding to"
      <<" database"<<endl
      <<"                     in verbose or very_verbose" 
      <<" modes; defaults to"<<endl
      <<"                     \"DB Size: %d\\tStream: "
      <<"%s\\tPair Number: %p\\n\""<<endl;
  cout<<"-cof compare_out_format Format for output when comparing"
      <<" with database"<<endl
      <<"                     in verbose or very_verbose modes;"  
      <<" defaults to"<<endl
      <<"                     \"Pair Number: %p\\tStream"
      <<" Number: %s\\n\""<<endl;
  exit(-1);
}



