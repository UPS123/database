//　reference -> https://en.wikibooks.org/wiki/Algorithm_Implementation/Trees/B%2B_tree

#include <iostream>
#include <string>

class BPlusTree
{
private:
  const static int lnum = 2;
  const static int inum = 2;
  int depth;
  struct LeafNode
  {
    int keys[lnum];
    int nval[lnum];
    std::string strval[lnum];
  };
  struct InnerNode
  {
    int keys[inum];
    void *children[inum + 1];
  };

public:
  BPlusTree()
  {
    depth = 0;
  };
  ~BPlusTree(){};

  void insert(int key, int value)
  {
    bool was_split;
    if (depth == 0)
    {
    }
    else
    {
    }
  }

  void delete_leaf()
  {
  }

  void update()
  {
  }

  bool find()
  {
  }
};