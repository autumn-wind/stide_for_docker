#ifndef __FLEXITREE_H
#define __FLEXITREE_H

#include<vector>
#include<iostream>
#include<ostream>

using std::ostream;
using std::istream;
using std::vector;

class FlexiTreeNode;
class FlexiTree {
  private:
    FlexiTreeNode *children;
    int root;
    int id;
  public:
    void Write(ostream &s, int &depth) const;
    int Read(istream &s, int &depth);
    int OutputGraph(ostream &s) const;
    FlexiTree();
    FlexiTree(int d);
//    FlexiTree(const FlexiTree& ft);
    ~FlexiTree();
    void SetRoot(int d) {root = d;}
    int InsertSeq(const vector<int> &seq, int first, int last);
    int IsSeqInTree(const vector<int> &seq, int first, int last) const;
    int ComputeHDistForTree(vector<int> &seq, int first, int last) const;
    friend ostream &operator<<(ostream &s, const FlexiTree &tn);
    friend istream &operator>>(istream &s, FlexiTree &tn);
    int NumNodes() const;  // returns the number of nodes in the tree
    int NumLeaves() const; // returns the number of leaves in the tree, i.e num of distinct seqs
    int NumBranches() const; // returns the total # of branches, of all nodes
};

//===========================================================================
class SeqForest {
public:
    // this structure is a an array of N tree nodes, i.e. a tree for each value 
    // type
    vector<FlexiTree> trees;
    // this structure is to record what types of values actually occured - 
    // for efficiency, if there were actually fewer value types than 
    // specified in the config
    vector<int> trees_found;
    SeqForest(int max_trees);
    int IsSeqInForest(const vector<int> &seq, int seq_len) const;
};

//===========================================================================

#endif

