//　reference -> https://en.wikibooks.org/wiki/Algorithm_Implementation/Trees/B%2B_tree

#include <iostream>
#include <string>
using std::string;

class BPlusTree
{
private:
  const static unsigned lnum = 2;
  const static unsigned inum = 2;
  unsigned depth;
  void *root;
  void *current_leaf;

  struct LeafNode
  {
    unsigned num_keys = 0; //現在入ってる要素の数
    int keys[lnum];        //keyの格納場所
    int nval[lnum];        //int型のvalueの格納場所
    string strval[lnum];   //string型のvalueの格納場所
  };
  struct InnerNode
  {
    unsigned num_keys = 0;    //現在入ってる要素の数
    int keys[inum];           //keyの格納場所
    void *children[inum + 1]; //子のinnerまたはleafへのポインタの集合。ファンアウトみたいな？
  };

  struct InsertionResult
  {
    int key;     //insertしたkeyの値
    void *left;  //左に何がいるのか
    void *right; //右に何がいるのか
  };

  unsigned leaf_position_for(const int &key, const int keys[], unsigned num_keys)
  {
    //leaf内にある複数の値の中から特定のある値の場所を見つける（orなければある値を入れたい場所を見つける）
    //二分探索で高速化もありだが別になくてもいい気がするので線形探索
    unsigned k = 0;
    while ((k < num_keys) && (keys[k] < key))
      ++k;
    return k;
  }

  unsigned inner_position_for(const int &key, const int keys[], unsigned num_keys)
  {
    //inner内にある複数の値の中から特定のある値の場所を見つける（orなければある値を入れたい場所を見つける）
    unsigned k = 0;
    while ((k < num_keys) && (keys[k] < key))
      ++k;
    return k;
  }

  static void leaf_insert_nonfull(LeafNode *node, int &key, int &nval, string &strval, unsigned index)
  {
    //分割しなくてもいけるとき、nodeに添字がindexの場所にkey, num, sを入れる
    if ((index < lnum) && (node->num_keys != 0) && (node->keys[index] == key))
    {
      //もしindexが容量より小さくて、既に何か入ってて、更新する場合
      node->nval[index] = nval;
      node->strval[index] = strval;
    }
    else
    {
      for (unsigned i = node->num_keys; i > index; --i)
      {
        //indexの場所を空けるために1個ずつ大きい方にずらしている
        node->keys[i] = node->keys[i - 1];
        node->nval[i] = node->nval[i - 1];
        node->strval[i] = node->strval[i - 1];
      }
      node->num_keys++;
      node->keys[index] = key;
      node->nval[index] = nval;
      node->strval[index] = strval;
    }
  }

  bool leaf_insert(LeafNode *node, int &key, int &nval, string &strval, InsertionResult *result)
  {
    //leafにkey,nval,strvalを挿入して結果をresultに。その時分割したかどうか返す
    bool was_split = false;                                          //そのnodeが容量overかどうかcheck
    unsigned i = leaf_position_for(key, node->keys, node->num_keys); //どこにならちょうどいい値に入るのか線形探索
    if (node->num_keys == lnum)
    {
      //nodeの容量がいっぱいなら
      unsigned threshhold = (lnum + 1) / 2;                //閾値というか要素数の真ん中の値（切り上げ）
      LeafNode *new_sibling = new LeafNode();              //分割したnodeのindexが大きい方（兄）
      new_sibling->num_keys = node->num_keys - threshhold; //さっき作った兄の要素数を入れる
      for (unsigned j = 0; j < new_sibling->num_keys; ++j)
      {
        //兄の方に入れる予定のkey, nval, strvalを移し替え
        new_sibling->keys[j] = node->keys[threshhold + j];
        new_sibling->nval[j] = node->nval[threshhold + j];
        new_sibling->strval[j] = node->strval[threshhold + j];
      }
      node->num_keys = threshhold; //分割されたnodeをindexが小さい方にする（弟）ので、要素数を変更
      // 分割したどちらに新しいleafの値を入れるか分岐
      if (i < threshhold)
        leaf_insert_nonfull(node, key, nval, strval, i);
      else
        leaf_insert_nonfull(new_sibling, key, nval, strval, i - threshhold);
      //分割したこと（境界値、左側、右側）をお知らせする
      was_split = true;
      result->key = new_sibling->keys[0]; //分割した時の境界値
      result->left = node;
      result->right = new_sibling;
    }
    else //分割しなくてもいいので
      leaf_insert_nonfull(node, key, nval, strval, i);
    return was_split;
  }

  void inner_insert_nonfull(InnerNode *node, unsigned depth, int &key, int nval, string strval)
  {
    //分割しなくてもいけるとき、nodeに添字がindexの場所にkey, num, sを入れる
    unsigned index = inner_position_for(key, node->keys, node->num_keys); //ちょうどいいところを選ぶ
    InsertionResult result;
    bool was_split;
    if (depth - 1 == 0)
    {
      //childrenがleafの時kleafは分割したのかcheckかつleafに値挿入
      was_split = leaf_insert(reinterpret_cast<LeafNode *>(node->children[index]), key, nval, strval, &result);
    }
    else
    {
      //再帰的にinnerに対して挿入していって、分割するのかcheck
      InnerNode *child = reinterpret_cast<InnerNode *>(node->children[index]);
      was_split = inner_insert(child, depth - 1, key, nval, strval, &result);
    }
    if (was_split)
    {
      if (index == node->num_keys)
      {
        // innerの一番右側に挿入する場合、leafまたはinner_insertで得られたresultから子へのポインタなどを入れる
        node->keys[index] = result.key;
        node->children[index] = result.left;
        node->children[index + 1] = result.right;
        node->num_keys++;
      }
      else
      {
        //間に入れるので、一個ずつ大きい方にずらしてから上と同様
        node->children[node->num_keys + 1] = node->children[node->num_keys];
        for (unsigned i = node->num_keys; i != index; --i)
        {
          node->children[i] = node->children[i - 1];
          node->keys[i] = node->keys[i - 1];
        }
        node->children[index] = result.left;
        node->children[index + 1] = result.right;
        node->keys[index] = result.key;
        node->num_keys++;
      }
    }
  }

  bool inner_insert(InnerNode *node, unsigned depth, int &key, int &nval, string &strval, InsertionResult *result)
  {
    //innerにkey,nval,strvalを挿入してそっからleafまで辿って行って結果をresultに。その時nodeを分割したかどうか返す
    bool was_split = false;
    if (node->num_keys == inum)
    {
      //nodeの容量がいっぱいなら
      unsigned threshhold = (inum + 1) / 2;                //閾値というか要素数の真ん中を設定（切り上げ）
      InnerNode *new_sibling = new InnerNode();            //分割したnodeのindexが大きい方（兄）
      new_sibling->num_keys = node->num_keys - threshhold; //さっき作った兄の要素数を入れる
      for (unsigned i = 0; i < new_sibling->num_keys; ++i)
      {
        //兄の方に入れる予定のkey, childrenを移し替え
        new_sibling->keys[i] = node->keys[threshhold + i];
        new_sibling->children[i] = node->children[threshhold + i];
      }
      new_sibling->children[new_sibling->num_keys] = node->children[node->num_keys]; //keyよりchildrenの方が1つ要素が多いので
      node->num_keys = threshhold - 1;                                               //分割されたnodeをindexが小さい方にする（弟）ので、要素数を変更
      //分割したこと（境界値、左側、右側）をお知らせする
      was_split = true;
      result->key = node->keys[threshhold - 1];
      result->left = node;
      result->right = new_sibling;
      // 分割したどちらに新しいinnerの値を入れるか分岐
      if (key < (result->key))
        inner_insert_nonfull(node, depth, key, nval, strval);
      else
        inner_insert_nonfull(new_sibling, depth, key, nval, strval);
    }
    else //nodeの容量がまだいけるので
      inner_insert_nonfull(node, depth, key, nval, strval);
    return was_split;
  }

public:
  BPlusTree()
  {
    depth = 0;
    root = new LeafNode();
  };
  ~BPlusTree(){};

  bool find(const int &key, int *nval = 0, string *strval = 0)
  {
    //keyがあるかcheck。またその値が欲しければ第2, 3引数に変数を入れる。
    InnerNode *inner;
    void *node = root;
    unsigned d = depth, index;
    while (d-- != 0)
    {
      //一番下（leaf）までkeyの位置に注意しながら降りていく
      inner = reinterpret_cast<InnerNode *>(node);
      index = inner_position_for(key, inner->keys, inner->num_keys);
      node = inner->children[index];
    }
    LeafNode *leaf = reinterpret_cast<LeafNode *>(node);
    index = leaf_position_for(key, leaf->keys, leaf->num_keys);
    if (leaf->keys[index] == key)
    {
      //もしkeyがあれば引数次第で値を代入
      if (nval != 0 && strval != 0)
      {
        *nval = leaf->nval[index];
        *strval = leaf->strval[index];
      }
      else if (nval != 0)
      {
        *nval = leaf->nval[index];
      }
      else if (strval != 0)
      {
        *strval = leaf->strval[index];
      }
      return true;
    }
    else
      return false;
  }

  void insert(int key, int nval, string strval)
  {
    //key, nval, strvalを挿入（または更新）
    InsertionResult result;
    bool was_split;
    if (depth == 0)
    {
      //rootがleafなら、そのままleafに挿入
      was_split = leaf_insert(reinterpret_cast<LeafNode *>(root), key, nval, strval, &result);
    }
    else
    {
      //rootがinnerなら、innerに代入して再帰的にchildrenを見て、innerになければ最終innerとleafに挿入。あればleafのみ更新する
      was_split = inner_insert(reinterpret_cast<InnerNode *>(root), depth, key, nval, strval, &result);
    }
    if (was_split)
    {
      //rootが分割されたら、新しくrootを作り直す
      depth++;                                                    //深さが1つ増えるので
      root = new InnerNode();                                     //rootを初期化
      InnerNode *rootProxy = reinterpret_cast<InnerNode *>(root); //rootをポインタ（代理）から書き換える
      rootProxy->num_keys = 1;                                    //現在新しく作られたところだから、要素は1つのみ
      //rootを分割するということは、resultで返ってきたやつは新しくrootに入るべきものなので
      rootProxy->keys[0] = result.key;
      rootProxy->children[0] = result.left;
      rootProxy->children[1] = result.right;
    }
  }

  unsigned sizeof_inner_node() const
  {
    //innerのメモリサイズ
    return sizeof(InnerNode);
  }

  unsigned sizeof_leaf_node() const
  {
    //leafのメモリサイズ
    return sizeof(LeafNode);
  }

  void delete_data(int &key)
  {
  }

  LeafNode select(int &minkey, int &maxkey)
  {
  }
};