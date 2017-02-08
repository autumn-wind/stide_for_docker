/*********************************************************************
 *                                                                   *
 * STIDE: Sequence Time-Delay Embedding v1.2                         *
 *                                                                   *
 * Written by Steve Hofmeyr 7/21/1996                                *
 * Revised by Julie Rehmeyer 3/1998                                  *
 * Revised by Hajime Inoue 11/2006                                   *
 *                                                                   *
 * Copyright (C) 1996, 1998 Regents of the University of New Mexico. *
 * Copyright (C) 2006 Hajime Inoue.                                  *
 * All Rights Reserved.                                              *
 *                                                                   *
 * This program is free software; you can redistribute it and/or     *
 * modify it under the terms of the GNU General Public License as    *
 * published by the Free Software Foundation; either version 2 of    *
 * the License, or (at your option) any later version.               *
 *                                                                   *
 * This program is distributed in the hope that it will be useful,   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License for more details.                      *
 *                                                                   *
 * You should have received a copy of the GNU General Public         *
 * License along with this program; if not, write to the Free        *
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,     *
 * USA.                                                              *
 *                                                                   *
 ********************************************************************/

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include <semaphore.h>

#include "config.h"
#include "stream.h"
#include "flexitree.h"
#include "syscall_info.h"
#include "sbuf.h"

#define DBREV 1

using std::vector;

using std::cin;
using std::cerr;
using std::cout;
using std::endl;
using std::ofstream;

typedef std::map<int, int> HashTableInt;

int counter = 0;

Stream *GetReadyStream(vector<Stream> &streams, HashTableInt
		&sid_table, int &num_streams_fnd, int
		&total_pairs_read, const Config &cfg);
int ReadDB(SeqForest &db_forest, const string &db_name, 
		int &seq_len); 
void WriteDB(const SeqForest &db_forest, const string &db_name, const
		int db_size, const int seq_len);
void FinalReport(const Config &cfg, const SeqForest &normal, const int
		num_streams_fnd, const int num_seqs_added, const
		vector<Stream> &streams, const int db_size); 
void WriteDBStats(const SeqForest &db_forest, ostream &out_stream,
		const int db_size); 
void OutputGraph(const SeqForest &db_forest, string db_name);
int GetPrimeLargerThan(const int n);


int ExtToInt(HashTableInt &sid_table, int key, int next_value)
{
	if(sid_table.find(key) == sid_table.end())
		sid_table[key] = next_value; 

	return sid_table[key];
}

/*********************************************************************
 *   main()                                                          *
 *   Input:  int argc:  Number of command-line arguments             *
 *           char *argv[]:  array of strings containing              *
 *                          command-line arguments                   *
 *   Output: 0 if successful, -1 if unsuccessful                     *
 *********************************************************************/

void* stide_for_docker(void *)

{
	Config cfg;    
	// Declare configuration object and do
	// the configuration on the basis of the
	// command line arguments and the
	// configuration file  
	Stream *active_stream;     // This will point to the stream that
	// currently has a sequence to be worked
	// on (either added to the database or
	// compared).   
	HashTableInt sid_table;
	// Hash table relating external stream ids to 
	// internal sids; make size of table
	// smallest prime larger than the number
	// of streams
	SeqForest normal(cfg.max_elements);     // Uninitialized forest of
	// normal sequences
	vector<Stream> streams(cfg.max_streams); // Array of stream objects,
	// one for each data stream
	// in input, which are
	// allocated as needed
	int num_streams_fnd = 0;                // Number of data streams
	// encountered to date
	int total_pairs_read = cfg.pair_offset; // Number of pairs read from
	// input to date from all
	// the data streams combined
	// -- can be offset using
	// the "-n" switch
	int db_size;                            // Total number of unique
	// sequences in the database
	int init_db_size = 0;                   // Number of unique
	// sequences in the
	// pre-existing database 




	// Read database into normal, if database exists
	db_size = init_db_size = ReadDB(normal, cfg.db_name, cfg.seq_len);

	if (cfg.add_to_db)
	{
		while ((active_stream = 
					GetReadyStream(streams, sid_table, num_streams_fnd,
						total_pairs_read, cfg)) != NULL)
		{ 
			active_stream->AddToDB(normal, db_size, total_pairs_read, cfg); 
		}
		WriteDB(normal, cfg.db_name, db_size, cfg.seq_len);
		if (cfg.output_graph)
		{
			OutputGraph(normal,cfg.db_name);
		}

	}
	else
	{
		int i = 0;
		while ((active_stream = 
					GetReadyStream(streams, sid_table, num_streams_fnd,
						total_pairs_read, cfg)) != NULL)
		{ 
			active_stream->CompareSeq(cfg, normal, total_pairs_read);
		}
	}

	FinalReport(cfg, normal, num_streams_fnd, db_size - init_db_size,
			streams, db_size); 

	return(0);
}

/**********************************************************************
 * GetReadyStream()                                                   *
 *   This function reads a pair from the input, appends the element   *
 *   to the current sequence string in the appropriate data stream,   *
 *   finds out if that data stream has a complete sequence to be      *
 *   processed, continues until it has found such a data stream, and  *
 *   returns a pointer to it.  It updates num_streams_fnd,            *
 *   total_pairs_read, sid_table, and streams.                        * 
 *                                                                    *
 * Input: vector<Stream> &streams: the array of streams that we have   *
 *                                found so far                        *
 *        HashTableInt &sid_table: hash table relating external sids  *
 *                                 to internal sids                   *
 *        int &num_streams_fnd: the number of streams found so far;   *
 *        int &total_pairs_read:  the number of pairs read from the   *
 *                                input stream so far                 *
 *        const Config &cfg: configuration information                *
 *                                                                    *
 * Output: a pointer to the next stream that is ready for processing  *
 **********************************************************************/
#define MAX_PARIS_LIM 100000

extern sbuf_t sbuf;

Stream *GetReadyStream(vector<Stream> &streams, HashTableInt
		&sid_table, int &num_streams_fnd, int
		&total_pairs_read, const Config &cfg)

{
	Stream *ready_stream = NULL;
	int ext_sid;
	int int_sid;
	int sval;

	//cin >> ext_sid;
	//while (!cin.eof()) {
		//if (ext_sid == -1) {
			//break;
		//}
		//    int_sid = sid_table.ExtToInt(ext_sid, num_streams_fnd);
	while(true) {

		if (total_pairs_read >= MAX_PARIS_LIM)
			break;
		
		//cin >> ext_sid;
		//cin >> sval;

        pro_con_bfr item;
        item = sbuf_remove(&sbuf);
		ext_sid = item.pid;
		sval = item.syscall_nr;

		int_sid = ExtToInt(sid_table, ext_sid, num_streams_fnd);
		++total_pairs_read;

		// Update num_streams_fnd, if necessary
		if (int_sid >= num_streams_fnd) {
			if (int_sid > cfg.max_streams) {
				cerr<<"ERROR: Too many streams to follow, aborting..."<<endl;
				exit(-1);
			}

			// We need a new stream object
			if(num_streams_fnd == streams.size())
			{
				cerr << "WRITING OVER THE END OF THE ARRAY" << endl;
				cerr << "num_streams_fnd: " << num_streams_fnd << endl;
				cerr << "cfg.max_streams: " << cfg.max_streams << endl;
				exit(-1);
			}
			streams[num_streams_fnd].Init(cfg, int_sid, ext_sid);
			num_streams_fnd = int_sid + 1;
		}
		streams[int_sid].Append(sval);
		if (streams[int_sid].Ready()) {
			ready_stream = &streams[int_sid];
			break;
		}
	}

	return ready_stream;
}

/*********************************************************************
 * ReadDB()                                                          *
 *   Reads the database from a file and returns the number of unique *
 *   sequences in the database.  Checks for appropriate revision     *
 *   number.  If it is a revision DBREV database, the second line    *
 *   will be "#DBseq_len: " followed by the sequence length.  The    *
 *   next line will contain a single number, giving the root of the  *
 *   first tree.  The following lines will contain the tree itself.  *
 *   The first seq_len numbers make up the first sequence (so the    *
 *   first number of the second line will be the same as the number  *
 *   on the first line).  The next number will be a negative number  *
 *   between -(seq_len-1) and -2, indicating how far to backtrack in *
 *   the first sequence, and the following positive numbers give the *
 *   rest of the second sequence.  So, for example, -3 would mean    *
 *   backtrack 3 numbers, take the previous numbers including the    *
 *   one you're on, and append the next two numbers.  So after the   *
 *   -3 you would find two positive numbers, followed by a negative  *
 *   number (which you would use the same way as you used the -3, on *
 *   the most recent sequence).  Each tree is terminated by the      *
 *   number -1.  So the sample input file                            *
 *     3                                                             *
 *     3 4 2 9 10 3 -4 3 9 8 -2 3 -3 4 9 -1                          *
 *     2                                                             *
 *     2 3 4 5 6 7 -3 2 9 -1                                         *
 *   yields the sequences:                                           *
 *     3 4 2 9 10 3                                                  *
 *     3 4 2 3 9 8                                                   *
 *     3 4 2 3 9 3                                                   *
 *     3 4 2 3 4 9                                                   *
 *     2 3 4 5 6 7                                                   *
 *     2 3 4 5 2 9                                                   *
 *                                                                   *
 *   Input:   SeqForest &db_forest   Forest of sequences             *
 *            const string &db_name  Name of database                *
 *            int &seq_len           User-specified sequence length  *
 *                                                                   *
 *   Output:  the number of unique sequences in the database         *
 *                                                                   *
 ********************************************************************/

int ReadDB(SeqForest &db_forest, const string &db_name, 
		int &seq_len)  
{
	ifstream in_db_file(db_name.c_str());  // file to read the database from
	int db_size = 0;               // size of the database
	int root;                      // the first element of the sequences
	// we are reading in at the moment;
	// i.e., the root of this tree
	string buff;
	int db_seq_len;
	int rev_num;

	if (!in_db_file.is_open()) {
		cerr<<"WARNING: Cannot open database file " << db_name 
			<< " for input"<<endl<<"Creating a new file"<<endl;
		return 0;
	} 

	// Check to see if the first line contains "#DBrev:"
	in_db_file>>buff;
	if (buff == "#DBrev:") {
		in_db_file>>rev_num;
		if (rev_num > DBREV) {
			cerr << "ERROR: The revision number is greater than " << DBREV
				<< ".  This version of STIDE is only capable of dealing "
				<< "with databases through DBrev " << DBREV 
				<< ".  Aborting..."<<endl; 
			exit(-1);
		}
		if (rev_num < DBREV) {
			cerr << "ERROR: Revision number of database must be >= " << DBREV 
				<< endl;
			exit(-1);
		}
		// Now we know that it is revision DBREV.  Check sequence length of
		// database against user-indicated sequence length 
		in_db_file>>buff;
		// Now check to see if next line is "#DBseq_len: " followed by a
		// number 
		if (buff != "#DBseq_len:") {
			cerr << "ERROR: The second line of the database does not "
				<< "contain the string \"#DBseq_len: \"" << endl
				<< "followed by the sequence length of the database, as "
				<< "required of revision " << DBREV 
				<< " databases.  Aborting..."<< endl; 
			exit(-1);
		}
		in_db_file>>db_seq_len;
		if (db_seq_len != seq_len) {
			cerr << "WARNING: Database sequence length is " << db_seq_len 
				<< ", which does not match "
				<< "sequence length specified" << endl 
				<< "by user (or by default if no specification was given), "
				<< "which is " << seq_len << endl
				<< "I will use the database sequence length.  If that is "
				<< "not what you intended, type Ctrl-C to abort." << endl;
			seq_len = db_seq_len;
		}
		// Read next number into root
		in_db_file >> root;
	}
	// Otherwise, we assume we have an old-style database, and let the
	// user know that that's our assumption
	else {
		cerr << "WARNING: The string \"DBrev: \" is not in the first "
			<< "line of the database." << endl
			<< "I'm assuming that it's an older style of database, and "
			<< "will read it in" << endl
			<< "based on that assumption.  If that is not what you want " 
			<< "me to do, type CTRL-C" << endl << endl;
		// we have just read the first root into buff -- put it in root
		// instead 
		root = atoi(buff.c_str());
	}

	while (!in_db_file.eof()) {
		if (root == -1) break;
		db_forest.trees_found[root]++; 
		in_db_file>>db_forest.trees[root];
		db_size += db_forest.trees[root].NumLeaves();
		in_db_file>>root;
	}
	in_db_file.close();

	return db_size;
}


/*********************************************************************
 * WriteDB()                                                         *
 *   Writes db_forest to the file db_name, with the format described *
 *   in the header of ReadDB().  Prints database statistics at the   *
 *   end of the file.                                                *
 *                                                                   *
 *   Input:  const SeqForest &db_forest  Forest of sequences in      *
 *                                       database                    * 
 *           const string &db_name        Name of file in which to    *
 *                                       put database.               *
 *           const int db_size           Number of unique sequences  *
 *                                       in the database             *
 *           const int seq_len           Sequence length             *
 *                                                                   *
 *   Output: none                                                    *
 ********************************************************************/

void WriteDB(const SeqForest &db_forest, const string &db_name, const
		int db_size, const int seq_len) 
{
	ofstream out_db_file(db_name.c_str());

	if (!out_db_file.is_open())
	{
		cerr << "ERROR: Cannot open database file " << db_name
			<< "for output, aborting..." << endl ;
		exit(-2);
	}

	out_db_file << "#DBrev: " << DBREV << endl;
	out_db_file << "#DBseq_len: " << seq_len << endl;

	for (int i = 0; i < db_forest.trees.size(); i++)
	{
		if (db_forest.trees_found[i])
		{
			out_db_file<<i<<endl;
			out_db_file << db_forest.trees[i] << endl;
		}
	}

	out_db_file<<" -1"<<endl;
	// we can now write anything, so I will write the db stats
	out_db_file<<"; DB STATS"<<endl;
	WriteDBStats(db_forest, out_db_file, db_size);
	out_db_file.close();
}

/*********************************************************************
 * FinalReport()                                                     *
 *   Reports data at end of run.  The number of streams, the number  *
 *   of input pairs, and the number of sequences in the input are    *
 *   always reported.  If we have done a comparison run, we report   *
 *   the number of anomalies, and the precentage of sequences that   *
 *   were anomalous.  Additionally, if asked for, the Hamming        *
 *   distance or locality frame count is reported.  If we have added *
 *   to the database, we report having done so and report the number *
 *   of sequences added.  If database statistics are asked for, we   *
 *   report the number of nodes, the number of unique sequences, the *
 *   number of branches, and the average database branch factor.     * 
 *                                                                   *
 *   Input:  const Config &cfg:         Configuration information    *
 *           const SeqForest &normal:   DB of normal sequences       *
 *           const int num_streams_fnd: Total number of streams found*
 *           const int num_seqs_added:  Number of unique sequences   *
 *                                      added                        *
 *           const vector<Stream> &streams:  Array of data streams    *
 *           const int db_size:         Number of unique sequences   *
 *                                      in DB                        *
 *                                                                   *
 *   Output: none                                                    *
 *                                                                   *
 *********************************************************************/

void FinalReport(const Config &cfg, const SeqForest &normal, const int
		num_streams_fnd, const int num_seqs_added, const
		vector<Stream> &streams, const int db_size) 
{
	int total_pairs = 0;
	int total_seqs = 0;
	int total_anoms = 0;
	int total_max_lfc = 0;
	int total_max_hdist = 0;
	int db_nodes = 0;
	int db_seqs = 0;
	int db_branches = 0;
	int j;

	// Sum up number of pairs input and number of seqs from all the streams
	for (j = 0; j < num_streams_fnd; j++) {
		total_seqs += streams[j].GetNumSeqsFnd();
		total_pairs += streams[j].GetNumPairsRead();
	}

	cout << endl;
	cout << "Number of different streams in input = " 
		<< num_streams_fnd << endl;
	cout << "Total number of input pairs = " 
		<< total_pairs << endl;
	cout << "Total number of sequences in input = " 
		<< total_seqs << endl;

	if (cfg.add_to_db) {
		cout << "File added to database" << endl;
		cout << "Number of new sequences added to the database: " 
			<< num_seqs_added << endl;
	}
	else {
		cout << "Scan completed" << endl;
		// Sum up number of anomalies from all the streams
		for (j = 0; j < num_streams_fnd; j++) {
			total_anoms += streams[j].GetNumAnoms();
		}

		cout << "Number of anomalies = " 
			<< total_anoms << endl;
		cout << "Percentage anomalous = " 
			<< ((float)total_anoms * 100.0)/total_seqs << endl;

		// If asked for, compute Hamming distances across streams and report
		if (cfg.compute_hdist) {
			for (j = 0; j < num_streams_fnd; j++) {
				if (streams[j].GetMaxHDist() > total_max_hdist) {
					total_max_hdist = streams[j].GetMaxHDist();
				}
			}
			cout << "Largest minimum Hamming distance = "
				<< total_max_hdist << endl; 
		}

		// If asked for, compute lfc across streams and report
		if (cfg.lf_size > 1) {
			for (j = 0; j < num_streams_fnd; j++) {
				if (streams[j].GetMaxLFC() > total_max_lfc) {
					total_max_lfc = streams[j].GetMaxLFC();
				}
			}
			cout << "Maximum lfc = " << total_max_lfc << endl;
		}
	}

	// If asked for, compute db stats and report
	if (cfg.write_db_stats) {
		WriteDBStats(normal, cout, db_size);
	}
}


/*********************************************************************
 * WriteDBStats()                                                    *
 *   Computes and writes to standard output the number of nodes in   *
 *   the database, the number of unique sequences, the number of     *
 *   branches, and the average database branch factor.               * 
 *                                                                   *
 *   Input:  const SeqForest &db_forest  Forest of sequences in      *
 *                                database                           *
 *           ostream &out_stream  Where to write info                *
 *           const int db_size    Number of unique sequences in the  *
 *                                database                           *
 *                                                                   *
 *   Output: none                                                    *
 *********************************************************************/

void WriteDBStats(const SeqForest &db_forest, ostream &out_stream,
		const int db_size)   
{
	int db_nodes = 0;
	int db_branches = 0;

	for (int i = 0; i < db_forest.trees.size(); i++)
	{
		if (db_forest.trees_found[i])
		{
			db_nodes += db_forest.trees[i].NumNodes();
			db_branches += db_forest.trees[i].NumBranches();
		}
	}

	out_stream << "Number of DB nodes = " << db_nodes << endl;
	out_stream << "Number of unique sequences = "<<db_size << endl;
	out_stream << "Number of branches (edges) = "<<db_branches << endl;
	out_stream << "Average DB branch factor = "  
		<<((float)db_branches/(db_nodes - db_size))<<endl;

}


/*********************************************************************
 * OutputGraph()                                                     *
 *   Writes a file db_name.dot containing input for the program Dot. *
 *   Running Dot on db_name.dot produces a PostScript file           *
 *   containing a picture of the whole database tree.                * 
 *                                                                   *
 *   Input:  const SeqForest &db_forest  Forest of sequences in      *
 *                                       database                    *
 *           const string db_name        Filename to use             *
 *                                                                   *
 *   Output: none                                                    *
 *********************************************************************/

void OutputGraph(const SeqForest &db_forest, const string db_name) 
{
	char *dot_filename;
	dot_filename = new char [strlen(db_name.c_str())+4];
	strcpy(dot_filename, db_name.c_str());
	ofstream output_file(strcat(dot_filename,".dot"));

	output_file<<"digraph \""<<db_name<<"\" {"<<endl;
	output_file<<"  ratio=auto;"<<endl;
	output_file<<"  page=\"8.5,11\";"<<endl;
	for (int i = 0; i < db_forest.trees.size(); i++) {
		if (db_forest.trees_found[i]) 
			db_forest.trees[i].OutputGraph(output_file);
	}
	output_file<<"}"<<endl;
	output_file.close();
}

