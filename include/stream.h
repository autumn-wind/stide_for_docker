#ifndef __STREAM_H
#define __STREAM_H

#include <vector>
#include "config.h"
#include "flexitree.h"

using std::vector;

class Stream {
  public:
    Stream() {};
    void Init(const Config &cfg, const int intern_id, const int
	      extern_id); 
    void Append(const int next_value);
    int AddToDB(SeqForest &normal, int &db_size, int total_pairs_read,
		const Config &cfg) const;  
    int CompareSeq(const Config &cfg, const SeqForest &normal, const 
		    int total_pairs_read); 
    int GetMaxHDist(void) const {return max_hdist;}
    int GetMaxLFC(void) const {return max_lfc;}
    int Ready(void) const {return ready;}
    int GetNumAnoms(void) const {return num_anoms;}
    int GetNumPairsRead(void) const {return num_pairs_read;}
    int GetNumSeqsFnd(void) const {return num_seqs_fnd;}
  private:
    static int valid_seq_num;
    static int last_num_of_uniques;
    static int num_of_uniques;
    static int last_num_of_anoms;
    static int num_of_anoms;
    static int counter_of_successive_low_incresement;

    vector<int> current_seq; // current sequence being filled or
			    // processed 
    int num_in_seq;         // current_seq is full up through
			    // num_in_seq  
    int num_pairs_read;     // the number of input pairs belonging to 
			    // this stream that have been read so far 
    int num_anoms;          // the number of anomalies found so far
    int num_seqs_fnd;       // the number of (not necessarily unique)
			    // sequences belonging to this stream
			    // found so far  
    int ext_sid;            // the external stream id 
    int int_sid;            // the internal stream id 
    int max_hdist;          // the largest minimum Hamming distance
			    // found in this stream
    int seq_hdist;          // the minimum Hamming distance for
			    // current_seq 
    vector<int> lf;          // array for locality frame
    int seq_lfc;            // the locality frame count for this
			    // sequence 
    int max_lfc;            // the largest locality frame count
			    // encountered so far
    int ready;              // a flag to indicate whether this stream
			    // has a full sequence ready to be
			    // processed.   0 = no, 1 = yes.
    int seq_len;            // sequence length
    int ComputeMisses(const SeqForest &normal);
    void ComputeHDist(const SeqForest &normal);
    void ComputeLF(const int is_anom, const int lf_size);
    void ReportSeq(const Config &cfg, const int total_pairs_read,
		   const int is_anom) const; 
    void ReportNewSeq(const Config &cfg, const int total_pairs_read,
		      const int db_size) const;
};

#endif


