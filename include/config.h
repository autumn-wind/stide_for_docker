#ifndef __SEQ_CONFIG_H
#define __SEQ_CONFIG_H

#define CFREV 1

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "opt_info.h"

using std::vector;
using std::ifstream;

class Config {
  public:
    Config();  // Constructor; reads
				     // configuration file and command
				     // line arguments
    string cfg_name;                 // Name of configuration file
    string db_name;                  // Name of database
    int seq_len;                     // Sequence Length
    int max_elements;                // Maximum number of different
				     // data elements we may encounter
    int max_streams;                 // Maximum number of different
				     // streams we may encounter
    int pair_offset;                 // Number by which to offset
				     // num_pairs_read 
    string add_output_format;        // Format for verbose-mode output
				     // when adding to database
    string compare_output_format;    // Format for verbose-mode output
				     // when comparing with an
				     // existing database
    int lf_size;                     // Size of locality frames: 1
				     // effectively means don't
				     // compute locality frames
    int add_to_db;                   // Flag indicating that we should
				     // add to the database rather
				     // than make comparisons
    int output_graph;                // Output graphing information in
				     // Dot format
    int compute_hdist;               // Compute Hamming distance
    int write_db_stats;              // Write statistics about the
				     // database 
    int verbose;                     // Output information about each
				     // anomaly or each new sequence
				     // added to the database
    int very_verbose;                // Output information about each
				     // sequence encountered
    char fmt_str[10][50];            // String used for outputting
				     // information in verbose mode
    char write_val[7];               // Do we write the value? used
				     // with fmt_str 
    int num_fvars;                   // Number of format variables

    void InitOptArray(vector<OptInfo> &opt_array);
    void SetDefaults();
    void ReadCommandLine(const int argc, const char *argv[],
			       vector<OptInfo> &opt_array);
    void AssignValToVar(vector<OptInfo> &opt_array, const 
				string &var_val, const string
				&var_name, const int name_type);
    void ReadConfigFile(vector<OptInfo> &opt_array); 
    void ReadOldConfigFile(ifstream &cfg_file, 
				   vector<OptInfo> &opt_array);
    void InitOutputFormat();
    void CheckValues();
    void OuputConfigInfo(const vector<OptInfo> &opt_array) const;
    void WriteHelpInfo() const;
};

#endif
