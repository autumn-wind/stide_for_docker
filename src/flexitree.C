// flexitree.C
#include "flexitree.h"

#include<iostream>
#include<ostream>

extern int counter; 

using std::endl;
using std::cerr;

// data structures:
// node for a linked list
class FlexiTreeNode {
  public:
    FlexiTree *tree;   // the element at this node
    FlexiTreeNode *next; // pointer to the next node
    FlexiTreeNode(int root) {tree = new FlexiTree(root); next = NULL;}
};
//===========================================================================
FlexiTree::FlexiTree(void) {
  children = NULL;
  root = -1;
  id = counter;
  counter++;
}
//===========================================================================
FlexiTree::FlexiTree(int d) {
  children = NULL;
  root = d;
  id = counter;
  counter++;
}
//============================================================================
FlexiTree::~FlexiTree(void) {
  if (children) {
    FlexiTreeNode *temp_ptr = children->next, *next_temp_ptr;
    if (children->tree) delete children->tree;
    delete children;
    while (temp_ptr) {
      next_temp_ptr = temp_ptr->next;
      if (temp_ptr->tree) delete temp_ptr->tree;
      delete temp_ptr;
      temp_ptr = next_temp_ptr;
    }
  }
}

//============================================================================
int FlexiTree::NumNodes(void) const {
  int size = 1;
  if (children) {
    FlexiTreeNode *temp_ptr = children;
    while (temp_ptr) {
      size += temp_ptr->tree->NumNodes();      
      temp_ptr = temp_ptr->next;
    }
  }
  return size;
}
//============================================================================
int FlexiTree::NumLeaves(void) const {
  int size;
  if (children) {
    size = 0;
    FlexiTreeNode *temp_ptr = children;
    while (temp_ptr) {
      size += temp_ptr->tree->NumLeaves();      
      temp_ptr = temp_ptr->next;
    }
  } else size = 1;
  return size;
}
//============================================================================
int FlexiTree::NumBranches(void) const {
  int branches = 0;
  if (children) {
    FlexiTreeNode *temp_ptr = children;
    while (temp_ptr) {
      branches += (temp_ptr->tree->NumBranches() + 1);
      temp_ptr = temp_ptr->next;
    }
  }
  return branches;
}
/**********************************************************************
 * InsertSeq()                                                        *
 *   Inserts a sequence in this tree and returns 1 if the sequence    *
 *   begins with the root of this tree and the sequence isn't already *
 *   in this tree.  It returns -1 if the sequence doesn't begin with  *
 *   the root of this tree.  It returns 0 if the sequence was already *
 *   in this tree.  This function is recursive and only compares the  *
 *   portion of the sequence lying between the argument first and the *
 *   argument last.                                                   *  
 *                                                                    *
 *                                                                    *
 * Input:  const vector<int> &seq  Current sequence                   *
 *         int first              The first element of the sequence   *
 *                                to consider                         *
 *         int last               The length of the sequence          *
 *********************************************************************/

int FlexiTree::InsertSeq(const vector<int> &seq, int first, int last)
{
  // If the root of this tree isn't the same as the first element of
  // the sequence, return -1 to indicate that
  if (root != seq[first]) {
    return -1;  
  }

  first++; // shift the seq forward
  // If we have reached the end of the sequence now, we haven't added
  // anything to the tree, so we return 0 to indicate that it was
  // already there
  if (first > last) {  
    return 0; 
  }

  // If there are no children, create some with the correct root,
  // insert the sequence and return 1.
  if (!children) {
    children = new FlexiTreeNode(seq[first]);
    children->tree->InsertSeq(seq, first, last);
    return 1;
  }

  // The root agrees, we're not at the end, and there are children.
  // Now we want to know if the sequence is already in the children,
  // and if not, we want to find out and add it.
  FlexiTreeNode *temp_ptr = children;
  int flag;
  while (1) {
    flag = temp_ptr->tree->InsertSeq(seq, first, last);
    // If the sequence is new and gets added, return 1
    if (flag == 1) return 1;
    // If the sequence is old, return 0
    if (flag == 0) return 0;
    // Otherwise the new root of the sequence isn't the same as the
    // root of this child tree, so we will try the next one.  But
    // first, if this is the last child, we know it isn't in here, so
    // we will add it in and return 1
    if (temp_ptr->next == NULL) {
      temp_ptr->next = new FlexiTreeNode(seq[first]);
      temp_ptr->next->tree->InsertSeq(seq, first, last);
      return 1;
    }
    temp_ptr = temp_ptr->next;
  }
}

/*********************************************************************
 * IsSeqInTree()                                                     *
 *   Returns 1 if the sequence has a match within this tree and      *
 *   returns 0 otherwise.  This function is recursive and only       *
 *   compares the portion of the sequence lying between the argument *
 *   first and the argument last.                                    * 
 *                                                                   *
 *                                                                   *
 * Input:  vector<int> &seq    Current sequence                      *
 *         int first          The first element of the sequence to   *
 *                            consider                               *
 *         int last           The length of the sequence             *
 ********************************************************************/

int FlexiTree::IsSeqInTree(const vector<int> &seq, int first, int last) const
{
  // If the first element of the sequence isn't the same as the root
  // of this tree, then we know already that there isn't a match here,
  // so return 0.
  if (root != seq[first]) {
    return 0;
  }
  first++; // shift the seq forward

  // If we have reached the end of the sequence, then we have
  // found matches all the way along, so return 1 saying that this is
  // a match.
  if (first > last) {
    return 1;  
  } 

  // Now we want to find out if there is a match in any of the
  // subtrees below this tree.  The subtrees are contained in the
  // linked list children->next->next->...
  FlexiTreeNode *next_node = children;
  while (next_node != NULL) {
    if (next_node->tree->IsSeqInTree(seq, first, last)) {
      return 1;  //Found it!
    }
    next_node = next_node->next;
  }
  // Now we've been through all of the subtrees without finding a
  // match, so there aren't any matches.
  return 0;  
}
/*********************************************************************
 * ComputeHDistForTree()                                             *
 *   Reports the minimum number of mismatches with any sequence on   *
 *   this tree.  This is a highly compute-intensive method, because  *
 *   every path down the tree is followed. This function is          *
 *   recursive, and only compares the portion of the sequence lying  *
 *   between the argument first and the argument last.               *
 *                                                                   *
 *                                                                   *
 * Input:  vector<int> &seq    Current sequence                      *
 *         int first          The first element of the sequence to   *
 *                            consider                               *
 *         int last           The length of the sequence             *
 ********************************************************************/

int FlexiTree::ComputeHDistForTree(vector<int> &seq, int first, int
				   last) const
{

  int tot_misses = 0; 

  // If the first element of the sequence isn't the same as the root
  // of this tree, then every sequence on this tree will disagree with
  // the sequence here, so we increment tot_misses
  if (root != seq[first]) {
    tot_misses++;  
  }

  first++; // shift the seq forward
  if (first > last) { // reached the end of the seq
    return tot_misses;  // return a zero, i.e. no mismatches
  } 

  // Now we want to add to tot_misses the smallest number of
  // mismatches with any of this tree's subtrees.  This tree's
  // subtrees are in the linked list children->next->next->
  FlexiTreeNode *next_node = children;
  // last is the last element of the sequence, which is one less than
  // the number of elements in the sequence.  The most misses possible
  // is the number of elements in the sequence.
  int min_misses = last + 1;
  int misses;
  while (next_node != NULL) {
    misses = next_node->tree->ComputeHDistForTree(seq, first, last);
    if (misses < min_misses) {
      min_misses = misses;
    }
    next_node = next_node->next;
  }
  return (tot_misses + min_misses); 
}
//===========================================================================
// format for writing out: we do it df, each path is terminated by a negative number,
// which is -(the reqd backtrack length)-1. depth should start out as 0.
// the tree writing out will end with -1.
void FlexiTree::Write(ostream &s, int &depth) const {
  s<<root<<" ";
  FlexiTreeNode *temp_ptr = children;
  while (temp_ptr) {
    depth = 0;
    temp_ptr->tree->Write(s, depth);
    temp_ptr = temp_ptr->next;
    if (temp_ptr) s<<"-"<<(depth + 1)<<" ";
  }
  depth++; // now incr the count
}
//=============================================================================
ostream &operator<<(ostream &s, const FlexiTree &tree) {
  int depth = 0;
  tree.Write(s, depth);
  s<<" -1";  // we terminate with a -1
  return s;
}
//===========================================================================
// returns 0 if we have reached the end of the file, 1 otherwise
int FlexiTree::Read(istream &s, int &depth) {
  int next_num;
  if (s.eof()) return 0;
  s>>next_num;

  if (next_num == -1) return 0; // we have reached the end of the tree

  if (next_num >= 0) {
    children = new FlexiTreeNode(next_num);
    if (!children->tree->Read(s, depth)) return 0; 
    FlexiTreeNode *temp_ptr = children;
    while (depth == 0) {
      if (s.eof()) return 0;
      s>>next_num;
      if (next_num == -1) return 0; // we have reached the end of the tree
      temp_ptr->next = new FlexiTreeNode(next_num);
      temp_ptr = temp_ptr->next;
      if (!temp_ptr->tree->Read(s, depth)) return 0;
    }
  } else depth = (-1 * next_num) - 1;
  if (depth) depth--;
  return 1;
}
//=============================================================================
istream &operator>>(istream &s, FlexiTree &tree) {
  int next_num, depth = 0;
  s>>next_num;
  tree.SetRoot(next_num);
  tree.Read(s, depth);
  return s;
}
//===========================================================================
// writes out in the format that dot uses for dags
int FlexiTree::OutputGraph(ostream &s) const {
  // first write out the name of the tree
  s<<"  "<<id<<" [label=\""<<root<<"\",shape=plaintext];"<<endl;

  FlexiTreeNode *temp_ptr = children;
  int childid;
  while (temp_ptr) {
    childid = temp_ptr->tree->OutputGraph(s);
    s<<"  "<<id<<" -> "<<childid<<";"<<endl;
    temp_ptr = temp_ptr->next;
  }
  return id;
}

/*********************************************************************
 * IsSeqInForest()                                                   * 
 *   Searches through database forest to locate sequence.  Returns 1 *
 *   if it finds it, 0 otherwise                                     *
 *********************************************************************/

int SeqForest::IsSeqInForest(const vector<int> &seq, int seq_len) const
{
  // Have we ever seen a sequence starting with the same root?
  if (trees_found[seq[0]]) { 
    // Have we seen this precise sequence?
    return trees[seq[0]].IsSeqInTree(seq, 0, seq_len-1);
  }
  return 0;
}

SeqForest::SeqForest(int max_trees)
{
    trees = vector<FlexiTree>(max_trees);
    trees_found = vector<int>(max_trees, 0);
}


/*
#include "fstream.h"

// for test purposes
void main(void) {
  FlexiTree tree(1);
  vector<int> seq(10);

  // try out insert and write
  seq[0] = 1; seq[1] = 1; seq[2] = 2; seq[3] = 3;
  tree.SeqInsert(seq, 0, 3);
  cout<<"1123:"<<tree<<endl;
  seq[0] = 1; seq[1] = 1; seq[2] = 3; seq[3] = 5;
  tree.SeqInsert(seq, 0, 3);
  cout<<"1134:"<<tree<<endl;
  seq[0] = 1; seq[1] = 2; seq[2] = 2; seq[3] = 3;
  tree.SeqInsert(seq, 0, 3);
  cout<<"1223:"<<tree<<endl;
  seq[0] = 1; seq[1] = 2; seq[2] = 3; seq[3] = 3;
  tree.SeqInsert(seq, 0, 3);
  cout<<"1233:"<<tree<<endl;
  seq[0] = 1; seq[1] = 2; seq[2] = 3; seq[3] = 4;
  tree.SeqInsert(seq, 0, 3);
  cout<<"1234:"<<tree<<endl;
  seq[0] = 1; seq[1] = 2; seq[2] = 3; seq[3] = 4;
  tree.SeqInsert(seq, 0, 3);
  cout<<"1234:"<<tree<<endl;
  seq[0] = 1; seq[1] = 2; seq[2] = 1; seq[3] = 4;
  tree.SeqInsert(seq, 0, 3);
  cout<<"1214:"<<tree<<endl;

  // now try out search
  seq[0] = 1; seq[1] = 2; seq[2] = 1; seq[3] = 4;
  if (tree.SeqSearch(seq, 0, 3)) cout<<"found 1214"<<endl;
  else cout<<"could not find 1214"<<endl;
  seq[0] = 1; seq[1] = 2; seq[2] = 2; seq[3] = 4;
  if (tree.SeqSearch(seq, 0, 3)) cout<<"found 1224"<<endl;
  else cout<<"could not find 1224"<<endl;
  seq[0] = 1; seq[1] = 2; seq[2] = 4; seq[3] = 4;
  if (tree.SeqSearch(seq, 0, 3)) cout<<"found 1244"<<endl;
  else cout<<"could not find 1244"<<endl;
  seq[0] = 1; seq[1] = 1; seq[2] = 3; seq[3] = 5;
  if (tree.SeqSearch(seq, 0, 3)) cout<<"found 1134"<<endl;
  else cout<<"could not find 1134"<<endl;

  // try out insert and write with shorter and longer sequences
  seq[0] = 1; seq[1] = 3;
  tree.SeqInsert(seq, 0, 1);
  cout<<"13:"<<tree<<endl;
  seq[0] = 1; seq[1] = 1; seq[2] = 4;
  tree.SeqInsert(seq, 0, 2);
  cout<<"114:"<<tree<<endl;
  seq[0] = 1; seq[1] = 2; seq[2] = 3; seq[3] = 1;  seq[4] = 1; seq[5] = 2; seq[6] = 1; seq[7] = 4;
  tree.SeqInsert(seq, 0, 7);
  cout<<"12311214:"<<tree<<endl;

  if (tree.SeqSearch(seq, 0, 7)) cout<<"found 12311214"<<endl;
  else cout<<"could not find 12311214"<<endl;
  seq[0] = 1; seq[1] = 1; seq[2] = 5;
  if (tree.SeqSearch(seq, 0, 2)) cout<<"found 115"<<endl;
  else cout<<"could not find 115"<<endl;
  if (tree.SeqSearch(seq, 0, 1)) cout<<"found 11"<<endl;
  else cout<<"could not find 11"<<endl;

  ofstream outf("test.out");
  outf<<tree;
  outf.close();

//counter = 0;

  FlexiTree intree;
  ifstream inf("test.out");
  inf>>intree;
  inf.close();

  cout<<endl<<intree;

  seq[0] = 1; seq[1] = 2; seq[2] = 3; seq[3] = 1;  seq[4] = 1; seq[5] = 2; seq[6] = 1; seq[7] = 4;
  if (intree.SeqSearch(seq, 0, 7)) cout<<"found 12311214"<<endl;
  else cout<<"could not find 12311214"<<endl;
  seq[0] = 1; seq[1] = 1; seq[2] = 5;
  if (intree.SeqSearch(seq, 0, 2)) cout<<"found 115"<<endl;
  else cout<<"could not find 115"<<endl;
  if (intree.SeqSearch(seq, 0, 1)) cout<<"found 11"<<endl;
  else cout<<"could not find 11"<<endl;

}
*/

/*
int FlexiTree::Read(istream &s, int &depth) {
  int next_num, depth_decr = 0;
  s>>next_num;
  if (next_num == -1) return 0; // we have reached the end of the tree
  if (next_num >= 0) {
    children = new FlexiTreeNode(next_num);
    if (!children->tree->Read(s, depth)) return 0;
    if (depth) {
      depth--;
      depth_decr = 1;
    }
    FlexiTreeNode *temp_ptr = children;
    while (depth == 0) {
      depth_decr = 0;
      s>>next_num;
      if (next_num == -1) return 0; // we have reached the end of the tree
      temp_ptr->next = new FlexiTreeNode(next_num);
      temp_ptr = temp_ptr->next;
      if (!temp_ptr->tree->Read(s, depth)) return 0;
      if (depth) {
        depth--;
        depth_decr = 1;
      }
    }
    if (!depth_decr && depth) depth--;
  } else
    depth = (-1 * next_num) - 1;
  return 1;
}
*/

