#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>

#include <sys/times.h>
#include <unistd.h>

#include "stream.h"
#include "common.h"

/********************************************************************
 *  Init()                                                          *
 *    Initializes an instance of Stream.                            *
 *                                                                  *
 *  Input:  const Config &cfg   Configuration information           *
 *          const int intern    internal stream identifier          *
 *          const int extern    external stream identifier          *
 *  Output: none                                                    *
 *******************************************************************/

using std::cerr;
using std::endl;
using std::cout;

int Stream::valid_seq_num = 0;
int Stream::last_num_of_uniques = 0;
int Stream::num_of_uniques = 0;
int Stream::last_num_of_anoms = 0;
int Stream::num_of_anoms = 0;
int Stream::counter_of_successive_low_incresement = 0;

void Stream::Init(const Config &cfg, 
        const int intern_id, const int extern_id) {
    int i;
    // initialize all the arrays
    current_seq.clear();
    current_seq.reserve(cfg.seq_len);
    for(i=0; i < cfg.seq_len; i++)
        current_seq[i] = -1;

    num_in_seq = -1;
    num_pairs_read = 0;
    num_anoms = 0;
    num_seqs_fnd = 0;
    int_sid = intern_id;
    ext_sid = extern_id;
    max_hdist = 0;
    seq_hdist = 0;
    lf.reserve(cfg.lf_size);
    for(i=0; i < cfg.lf_size; i++)
        lf[i] = 0;
    seq_lfc = 0;
    max_lfc = 0;
    ready = 0;
    seq_len = cfg.seq_len;
}

/*********************************************************************
 * Append()                                                          *
 *   This function puts the integer given into the current_seq array *
 *   as the last element.  It flags ready according to whether       *
 *   current_seq is full.  Updates num_in_seq, ready, current_seq,   *
 *   num_seqs_fnd, and num_pairs_read.                               *
 *                                                                   *
 *  Input:  const int new_value  The next value to be put into the   *
 *                          current_seq array                        *
 *  Output: none                                                     *
 ********************************************************************/

void Stream::Append(const int new_value)
{
    // missing system call - zero the current sequence 
    if (new_value == -1) {    
        num_in_seq = -1;
        ready = 0;
    } 
    else {
        num_pairs_read++;
        if (num_in_seq < seq_len - 1) {  // window not yet full
            num_in_seq++; 
            current_seq[num_in_seq] = new_value;  
            if (num_in_seq == seq_len - 1) {
                ready = 1;
                ++num_seqs_fnd;
            }
        } 

        else { 
            // Roll over current_seq array
            for (int k = 0; k < num_in_seq; k++) {
                current_seq[k] = current_seq[k + 1];
            }
            current_seq[num_in_seq] = new_value;  
            ++num_seqs_fnd;
        }
    }
}

/********************************************************************
 * AddToDB()                                                        *
 *                                                                  *
 *    Adds current_seq to the database if it isn't already there;   *
 *    Returns 0 if it is already there, 1 if it is new.  Updates    * 
 *    normal and db_size.                                           * 
 *                                                                  *
 * Input: SeqForest &normal  Forest of normal sequences             *
 *        int &db_size       Number of unique sequences in the      *
 *                           database                               *
 *        const int total_pairs_read  Number of pairs read from the *
 *                           entire input stream                    *
 *        const Config &cfg  Configuration Information              *
 * Output: 0 if sequence isn't new, 1 if it is                      *
 ********************************************************************/

#define SEC_PER_FIVE_MIN 300
#define SEC_PER_MIN 60
#define SEC_PER_HOUR 3600

int Stream::AddToDB(SeqForest &normal, int &db_size, const int
        total_pairs_read, const Config &cfg) const 
{
    int is_new;

    // If there is not a tree with the same root as this sequence has,
    // make a new tree with that root and flag trees_found
    if (!normal.trees_found[current_seq[0]])
    {
        normal.trees[current_seq[0]].SetRoot(current_seq[0]); 
        normal.trees_found[current_seq[0]] = 1;
    } 

    // Try to add the sequence.  If it's already there, is_new will be
    // set to 0, otherwise it will be set to 1.
    is_new = normal.trees[current_seq[0]].InsertSeq(current_seq, 0, seq_len-1);  
    db_size += is_new;

    valid_seq_num += 1;
    num_of_uniques += is_new;

    static clock_t first_time = times(NULL);
    if(first_time == -1)
        err_EXIT("times error");

    static long clktck = 0;
    if(clktck == 0) {
        if((clktck = sysconf(_SC_CLK_TCK)) < 0)
            err_EXIT("sysconf error");
    }

    static long time_to_reach = 0;

    if(time_to_reach >= 2 * SEC_PER_HOUR) {
        return 2;
    }

    long cur_time = times(NULL);
    if((cur_time - first_time) / (double)clktck >= time_to_reach) {
        cout << time_to_reach / SEC_PER_MIN << " min:\t" << total_pairs_read << "\t" << num_of_uniques << endl;
        time_to_reach += SEC_PER_MIN;
    }

     //when we read 4K valid_seqs, we should count how many unique seqs we have and calculate the increasement
    //if(( valid_seq_num & 0xFFF ) == 0) {
        //if(num_of_uniques == last_num_of_uniques) {
            //counter_of_successive_low_incresement += 1;
             //if in successive 10 times, the increment is 0, we should stop the monitoring
            //if(counter_of_successive_low_incresement >= 10) {
                //return 2;
            //}
        //}
         //if the increment is not 0, make counter_of_successive_low_incresement begin at 0
        //else {
            //counter_of_successive_low_incresement = 0;
        //}
        //last_num_of_uniques = num_of_uniques;
        //cout << counter_of_successive_low_incresement << "  " << last_num_of_uniques << endl;
    //}

    //if ((is_new && cfg.verbose) || cfg.very_verbose)
    //{
        //ReportNewSeq(cfg, total_pairs_read, db_size);
    //} 

    //if (is_new) 
        //return 1;
    //else 
        return 0;
} 


/*********************************************************************
 *  CompareSeq()                                                     *
 *    Compares the current sequence in this stream to the database,  *
 *    in the manner indicated by the configuration file.  Reports    *
 *    on anomalies if told to by the configuration file.  Updates    *
 *    num_anoms, seq_hdist, max_hdist, seq_lfc, and max_lfc.         *
 *                                                                   *
 *    Input:  const Config &cfg: Information from configuration file *
 *            const SeqForest &normal: DB of normal sequences        *
 *            const int total_pairs_read: Number of pairs read from  *
 *                                  all of the streams               *
 *    Output: none                                                   *
 ********************************************************************/

extern string CID;

int Stream::CompareSeq(const Config &cfg, const SeqForest &normal,
        const int total_pairs_read) 
{
    int is_anom;     // flag to indicate whether current_seq is an anomaly 

    is_anom = ComputeMisses(normal);

    valid_seq_num += 1;
    num_of_anoms += is_anom;

    if((valid_seq_num & 0xFF) == 0) {
        cout << num_of_anoms - last_num_of_anoms << endl;
        if(num_of_anoms - last_num_of_anoms >= 25) {
            cout << "Alarm! The container may already been hacked!" << endl
            << "Going to stop the whole container." << endl
            << "Local mismatch rate: " << (num_of_anoms - last_num_of_anoms) * 100.0 / 0xFF
            << "%" << endl;
            //cout << last_num_of_anoms << "  " << num_of_anoms << "  " << num_of_anoms - last_num_of_anoms << endl;
            string cmd("docker stop ");
            cmd += CID;
            char *const stop_container_cmd = new char[cmd.length() + 1];
            strcpy(stop_container_cmd, cmd.c_str());
            system(stop_container_cmd);
            cout << "The container " << CID << " has been stopped." << endl;
            exit(1);
        }
        last_num_of_anoms = num_of_anoms;
    }

    //if ((is_anom) && (cfg.compute_hdist)) {
        //ComputeHDist(normal);
    //}
    //if (cfg.lf_size > 1) {
        //ComputeLF(is_anom, cfg.lf_size);
    //}
    //// if we're in verbose mode and either current_seq is an anomaly or
    //// its locality frame contains an anomaly, report it
    //if ((cfg.very_verbose) || (cfg.verbose && (is_anom || seq_lfc))) {
        //ReportSeq(cfg, total_pairs_read, is_anom);
    //}
    return 1;
}

/*********************************************************************
 *  ComputeMisses()                                                  *
 *    Compares the current sequence to the database sequences.  If   *
 *    there is an exact match, we return 0.  Otherwise we return 1.  *
 *    Updates num_anoms and seq_hdist.                               *
 *                                                                   *
 *    Input:  const SeqForest &normal:  DB of normal sequences       *
 *    Output: 0 if there is an exact match                           *
 *            1 if the sequence is anomalous                         *
 ********************************************************************/


int Stream::ComputeMisses(const SeqForest &normal)
{
    if (normal.IsSeqInForest(current_seq, seq_len)) {
        seq_hdist = 0;
        return(0);
    }

    // We have an anomaly
    ++num_anoms;
    return(1);
} 

/*********************************************************************
 *  ComputeHDist()                                                   *
 *    Compares the current sequence in this stream to each sequence  *
 *    in the database in turn, adding up the number of mismatches    *
 *    between the two sequences.  The smallest difference between    *
 *    the current sequence and the database sequences is the minimum *
 *    Hamming distance for the current sequence.  If this minimum    *
 *    Hamming distance is greater than the largest minimum Hamming   *
 *    distance encountered so far, then the variable max_hdist is    *
 *    updated.  Updates seq_hdist and max_hdist.                     *
 *                                                                   *
 *    Input:  const SeqForest &normal: DB of normal sequences        *
 *                                                                   *
 *    Output: none                                                   *
 ********************************************************************/

void Stream::ComputeHDist(const SeqForest &normal)
{
    int misses_on_this_seq;    // the number of mismatches between
    // current_seq and the sequence we're
    // comparing it with at the moment
    seq_hdist = seq_len;       // start with seq_hdist as high as
    // possible  

    // We compare current_seq with each sequence in our database tree
    for (int i = 0; i < normal.trees.size(); i++) { 
        // Have we seen any sequences starting with element i?  If not, we
        // can go on to consider sequences starting with element i+1.
        if (normal.trees_found[i]) { 
            misses_on_this_seq =
                normal.trees[i].ComputeHDistForTree(current_seq, 0, seq_len-1);  
            if (misses_on_this_seq < seq_hdist) {
                seq_hdist = misses_on_this_seq;
            }
        }
    }  

    if (seq_hdist > max_hdist) {
        max_hdist = seq_hdist;
    }
} 


/*********************************************************************
 *  ComputeLF()                                                      *
 *    Computes the number of misses in current_seq's locality frame. *
 *    Updates lf, seq_lfc and max_lfc.                               *
 *                                                                   *
 *    Input:  const int is_anom   Flag to indicate whether           *
 *                          current_seq is an anomaly                *
 *            const int lf_size   Size of locality frame             *
 *    Output: none                                                   *
 ********************************************************************/


void Stream::ComputeLF(const int is_anom, const int lf_size)
{
    // When num_seqs_fnd is less than lf_size, the locality frame
    // array is not full
    if (num_seqs_fnd <= lf_size) {
        lf[num_seqs_fnd-1] = is_anom;
        seq_lfc += is_anom;
    }
    else {
        // We're about to remove the first element of lf; since seq_lfc is
        // the sum of the elements of lf, we should subtract lf[0] from
        // seq_lfc to remove it from the sum.
        seq_lfc -= lf[0];
        // Now we add is_anom and seq_lfc is the sum of the new locality
        // frame.
        seq_lfc += is_anom;

        // roll over the array
        for (int i = 0; i < lf_size-1; i++) {
            lf[i] = lf[i+1];
        }
        lf[lf_size-1] = is_anom;
    }
    if (seq_lfc > max_lfc) {
        max_lfc = seq_lfc;
    }
}

/*********************************************************************
 *  ReportSeq()                                                      *
 *    This function reports data about a sequence.  Specifically, it *
 *    can report the external stream id, a number indicating where   *
 *    the first element of the current sequence occurs in the input, *
 *    a number indicating how many pairs from this particular data   *
 *    stream have been read prior to the first element of the        *
 *    sequence, the minimum Hamming distance for the current         *
 *    sequence, the locality frame count, the locality frame count,  *
 *    and whether this particular sequence is itself an anomaly (it  *
 *    could be that some other sequence in its locality frame is     *
 *    anomalous).  The configuration file determines which of those  *
 *    possible data are reported and in what format. Updates no      *
 *    values.                                                        *
 *                                                                   *
 *    Input:  const Config &cfg  Configuration information           *
 *            const int total_pairs_read  Total number of pairs read *
 *                               from the input stream from any data *
 *                               stream, not just this one           *
 *            const int is_anom  flag for whether the current        *
 *                               sequence is itself an anomaly       * 
 *    Output: none                                                   *
 ********************************************************************/

void Stream::ReportSeq(const Config &cfg, const int total_pairs_read,
        const int is_anom)   const
{
    for (int i = 0; i < cfg.num_fvars; i++) {
        switch (cfg.write_val[i]) {
            case 'a':
                printf(cfg.fmt_str[i], is_anom); break;
            case 'c':
                if (cfg.lf_size > 1) {
                    printf(cfg.fmt_str[i], seq_lfc); 
                }
                break;
            case 'h':
                if (cfg.compute_hdist) {
                    printf(cfg.fmt_str[i], seq_hdist); 
                }
                break;
            case 'i': 
                printf(cfg.fmt_str[i], num_pairs_read); break;
            case 'p':
                printf(cfg.fmt_str[i], total_pairs_read); break;
            case 's': 
                printf(cfg.fmt_str[i], ext_sid); break;
        }
    }
    printf(cfg.fmt_str[cfg.num_fvars]);
}


/*********************************************************************
 *  ReportNewSeq()                                                   *
 *    This function reports on sequences which have been newly added *
 *    to the database.  It can report the external stream            *
 *    identifier, where the first element of the sequence occurs     *
 *    both within the whole input stream and within its own data     *
 *    stream, and the number of unique sequences in the database     *
 *    after this sequence has been added.  The configuration file    *
 *    determines which of those possible data are reported and in    *
 *    what format.  Updates no values.                               * 
 *                                                                   *
 *    Input:  const Config &cfg  Configuration information           *
 *            const int total_pairs_read  Total number of pairs read *
 *                               from the input stream from any data *
 *                               stream, not just this one           *
 *            const int db_size  Number of unique sequences          *
 *                               in the database                     *
 *    Output: none                                                   *
 ********************************************************************/

void Stream::ReportNewSeq(const Config &cfg, const int total_pairs_read,
        const int db_size) const
{
    for (int i = 0; i < cfg.num_fvars; i++) {
        switch (cfg.write_val[i]) {
            case 'd':
                printf(cfg.fmt_str[i], db_size); break;
            case 'i': 
                printf(cfg.fmt_str[i], num_pairs_read); break;
            case 'p':
                printf(cfg.fmt_str[i], total_pairs_read); break;
            case 's': 
                printf(cfg.fmt_str[i], ext_sid); break;
        }
    }
    printf(cfg.fmt_str[cfg.num_fvars]);
}



