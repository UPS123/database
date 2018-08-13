//　reference -> https://en.wikibooks.org/wiki/Algorithm_Implementation/Trees/B%2B_tree

#include <iostream>
#include <vector>
#include <memory>
#include <string>

const size_t maxLeafNum = 2;
const size_t maxInnerNum = 2;
const size_t minLeafNum = (maxLeafNum + 1) / 2;
const size_t minInnerNum = (maxInnerNum + 1) / 2;

/*
* ノードの構造体(抽象型)
* @param 要素数確保
* @detail num_keys ノード内のキーの個数, keys キー配列
*/
struct NodeBase
{
  size_t num_keys;
  std::vector<size_t> keys;
  NodeBase(size_t num) : num_keys(0), keys(num) {}
  virtual ~NodeBase() {}
};

/*
* 葉ノードの構造体
* @param maxLeafNum 要素数確保
* @detail numval 数値配列, value 文字列配列
*/
struct LeafNode : NodeBase
{
  std::vector<std::string> value;
  LeafNode() : NodeBase(maxLeafNum), value(maxLeafNum) {}
  ~LeafNode() {}
};

/*
* 内部ノードの構造体
* @param maxInnerNum 要素数確保
* @detail childref 子ノードへの参照配列
*/
struct InnerNode : NodeBase
{
  std::vector<std::unique_ptr<NodeBase>> childref;
  InnerNode() : NodeBase(maxInnerNum), childref(maxInnerNum + 1) {}
  ~InnerNode() {}
};

/*
* 挿入/削除した結果を格納する構造体(必要？)
*/
struct InsertionResult
{
  size_t key;                      //insertしたkeyの値
  std::unique_ptr<NodeBase> left;  //左に何がいるのか
  std::unique_ptr<NodeBase> right; //右に何がいるのか
};

/*
* B+ツリークラス
*/
class BPlusTree
{
private:
  unsigned int depth;
  std::unique_ptr<NodeBase> root;

  /*
  * @breif 葉ノード内のキーのインデックスを返す
  * @param [const int&]key 探すキー
  * @param [vector<int>&]keys 葉ノード内のキー配列
  * @param [const int&]num_keys キー配列に格納されている数
  * @return キーのインデックス(なければそのキー以下のインデックス)
  * @note 二分探索で高速化もありだが別になくてもいい気がするので線形探索
  */
  unsigned leaf_position_for(const size_t &key, const std::vector<size_t> &keys, const size_t num_keys) const
  {
    size_t k = 0;
    while ((k < num_keys) && (keys[k] < key))
      ++k;
    return k;
  }

  /*
  * @breif 対象のキーに対する内部ノード内のキー配列のインデックスを返す
  * @param [const int&]key 対象のキー
  * @param [vector<int>&]keys 内部ノード内のキー配列
  * @param [const int&]num_keys 内部ノードに格納されている数
  * @return 対象キーのインデックス(なければそのキー以下のインデックス)
  * @note 二分探索で高速化もありだが別になくてもいい気がするので線形探索
  */
  unsigned inner_position_for(const size_t &key, const std::vector<size_t> &keys, const size_t num_keys) const
  {
    size_t k = 0;
    while ((k < num_keys) && (keys[k] < key))
      ++k;
    return k;
  }

  /*
  * @breif 分割せずに葉ノードに新しい値を挿入
  * @param [LeafNode*]node 挿入するキーがあるところの葉ノード
  * @param [const int&]key 挿入するキー
  * @param [const int&]numval 挿入する数値
  * @param [const string&]value 挿入する文字列
  * @param [const unsigned int]index キーを挿入する場所のインデックス
  */
  void leaf_insert_nonfull(std::unique_ptr<LeafNode> node, const size_t &key, const std::string &value, const size_t index)
  {
    // 更新するとき
    if ((index < maxLeafNum) && (node->num_keys != 0) && (node->keys[index] == key))
    {
      node->value[index] = value;
    }
    else
    {
      for (size_t i = node->num_keys; i > index; --i)
      {
        node->keys[i] = node->keys[i - 1];
        node->value[i] = node->value[i - 1];
      }
      node->num_keys++;
      node->keys[index] = key;
      node->value[index] = value;
    }
  }

  /*
  * @breif 葉ノードに新しい値を挿入
  * @param [LeafNode*]node 現在参照されている葉ノード
  * @param [const int&]key 挿入するキー
  * @param [const int&]numval 挿入する数値
  * @param [const string&]value 挿入する文字列
  * @param [InsertionResult*]result 分割した時の結果
  * @return [bool]was_split 挿入時分割したかどうか
  */
  bool leaf_insert(std::unique_ptr<LeafNode> node, const size_t &key, const std::string &value, std::unique_ptr<InsertionResult> result)
  {
    bool was_split = false;
    size_t i = leaf_position_for(key, node->keys, node->num_keys);
    if (node->num_keys == maxLeafNum)
    {
      unsigned threshold = minLeafNum;
      //分割したnodeのindexが大きい方（兄）
      std::unique_ptr<LeafNode> new_sibling = std::make_unique<LeafNode>();
      new_sibling->num_keys = node->num_keys - threshold;
      for (unsigned int j = 0; j < new_sibling->num_keys; ++j)
      {
        //兄の方に入れる予定のkey, numval, valueを移し替え
        new_sibling->keys[j] = node->keys[threshold + j];
        new_sibling->value[j] = node->value[threshold + j];
      }
      node->num_keys = threshold;
      if (i < threshold)
        leaf_insert_nonfull(std::move(node), key, value, i);
      else
        leaf_insert_nonfull(std::move(new_sibling), key, value, i - threshold);
      //分割したこと（境界値、小さい方、大きい方）をお知らせする
      was_split = true;
      result->key = new_sibling->keys[0]; //分割した時の境界値
      result->left = std::move(node);
      result->right = std::move(new_sibling);
    }
    else //分割しなくてもいいので
      leaf_insert_nonfull(std::move(node), key, value, i);
    return was_split;
  }

  /*
  * @breif 内部ノードが全て埋まっていない場合の内部ノードに新しい値を挿入
  * @param [InnerNode*]node キーを挿入するところの内部ノード
  * @param [unsigned int]depth 内部ノードの深さ
  * @param [const int&]key 挿入するキー
  * @param [const int&]numval 挿入する数値
  * @param [const string&]value 挿入する文字列
  */
  void inner_insert_nonfull(std::unique_ptr<InnerNode> node, unsigned int depth, const size_t &key, const std::string value)
  {
    size_t index = inner_position_for(key, node->keys, node->num_keys);
    std::unique_ptr<InsertionResult> result;
    bool was_split;
    if (depth - 1 == 0)
    {
      //葉ノードに挿入
      was_split = leaf_insert(std::move(node->childref[index]), key, value, std::move(result));
    }
    else
    {
      //再帰的に内部ノードを走査する
      was_split = inner_insert(std::move(node->childref[index]), depth - 1, key, value, std::move(result));
    }
    if (was_split)
    {
      if (index == node->num_keys)
      {
        // 一番右側に挿入する場合
        node->keys[index] = result->key;
        node->childref[index] = std::make_unique<NodeBase>(result->left);
        node->childref[index + 1] = std::make_unique<NodeBase>(result->right);
        node->num_keys++;
      }
      else
      {
        //間に入れるので、一個ずつ大きい方に移動してしてから挿入
        node->childref[node->num_keys + 1] = std::move(node->childref[node->num_keys]);
        for (unsigned i = node->num_keys; i != index; --i)
        {
          node->childref[i] = std::move(node->childref[i - 1]);
          node->keys[i] = node->keys[i - 1];
        }
        node->childref[index] = std::make_unique<NodeBase>(result->left);
        node->childref[index + 1] = std::make_unique<NodeBase>(result->right);
        node->keys[index] = result->key;
        node->num_keys++;
      }
    }
  }

  /*
  * @breif 内部ノードに新しい値を挿入
  * @param [InnerNode*]node 現在参照している内部ノード
  * @param [unsinged int]depth 内部ノードが存在しているところの深さ
  * @param [const int&]key 挿入するキー
  * @param [const int&]numval 挿入する数値
  * @param [const string&]value 挿入する文字列
  * @param [InsertionResult*]result 分割した時の結果
  * @return [bool]was_split 挿入時分割したかどうか
  * @detail 新しい値を挿入する->経由する内部ノード内の数は増加するので、
  *         その時全て埋まっていたら分割する。
  */
  bool inner_insert(std::unique_ptr<InnerNode> node, unsigned int depth, const size_t &key, const std::string &value, std::unique_ptr<InsertionResult> result)
  {
    //innerにkey,numval,valueを挿入してそっからleafまで辿って行って結果をresultに。その時nodeを分割したかどうか返す
    bool was_split = false;
    if (node->num_keys == maxInnerNum)
    {
      //nodeの容量がいっぱいなら
      unsigned int threshold = minInnerNum;
      //分割したnodeのindexが大きい方（兄）
      std::unique_ptr<InnerNode> new_sibling;
      new_sibling->num_keys = node->num_keys - threshold;
      for (unsigned int i = 0; i < new_sibling->num_keys; ++i)
      {
        //兄の方に入れる予定のkey, childrefを移し替え
        new_sibling->keys[i] = node->keys[threshold + i];
        new_sibling->childref[i] = std::move(node->childref[threshold + i]);
      }
      new_sibling->childref[new_sibling->num_keys] = std::move(node->childref[node->num_keys]);
      node->num_keys = threshold - 1; //分割されたnodeをindexが小さい方にする（弟）ので、要素数を変更
      //分割したこと（境界値、その小さい方、大きい方）をお知らせする
      was_split = true;
      result->key = node->keys[threshold - 1];
      result->left = std::move(node);
      result->right = std::move(new_sibling);
      // 分割したどちらに新しい値を挿入するか
      if (key < (result->key))
        inner_insert_nonfull(std::move(node), depth, key, value);
      else
        inner_insert_nonfull(std::move(new_sibling), depth, key, value);
    }
    else //nodeの容量がまだいけるので
      inner_insert_nonfull(std::move(node), depth, key, value);
    return was_split;
  }

  /*
  * @brief 合併せずに葉ノード内の対象のキー、値を削除
  * @param [LeafNode*]node 
  * @param [int]key 削除するキー
  * @detail 
  */
  void leaf_delete_nonmerge(LeafNode *node, size_t &key)
  {
  }

  /*
  * @brief 葉ノード内の対象のキー、値を削除
  * @param [LeafNode*] 
  */
  bool leaf_delete(LeafNode *node)
  {
  }

  void inner_delete_nonmerge()
  {
  }

  bool inner_delete()
  {
  }

public:
  BPlusTree()
  {
    depth = 0;
    root = std::make_unique<NodeBase>();
  };
  ~BPlusTree(){};

  /*
  * @brief 新たな値を挿入/更新
  * @param [int]key
  * @param [int]numval
  * @param [string]value
  */
  void insert(size_t key, std::string value)
  {
    std::unique_ptr<InsertionResult> result;
    bool was_split;
    if (depth == 0)
    {
      //rootがleafなら、そのままleafに挿入
      was_split = leaf_insert(std::move(root), key, value, std::move(result));
    }
    else
    {
      //rootがinnerなら、innerに代入して再帰的にchildrefを見て、innerになければ最終innerとleafに挿入。あればleafのみ更新する
      was_split = inner_insert(std::move(root), depth, key, value, std::move(result));
    }
    if (was_split)
    {
      //rootが分割されたら、新しくrootを作り直す
      depth++;                                                      //深さが1つ増えるので
      root = std::make_unique<InnerNode>();                         //rootを初期化
      InnerNode *rootProxy = dynamic_cast<InnerNode *>(root.get()); //rootをポインタ（代理）から書き換える
      rootProxy->num_keys = 1;                                      //現在新しく作られたところだから、要素は1つのみ
      //rootを分割するということは、resultで返ってきたやつは新しくrootに入るべきものなので
      rootProxy->keys[0] = result->key;
      rootProxy->childref[0] = std::make_unique<NodeBase>(result->left);
      rootProxy->childref[1] = std::make_unique<NodeBase>(result->right);
    }
  }

  bool find(const size_t &key, std::string *value = nullptr)
  {
    //keyがあるかcheck。またその値が欲しければ第2, 3引数に変数を入れる。
    InnerNode *inner;
    NodeBase *node = root.get();
    size_t d = depth, index;
    while (d-- != 0)
    {
      //一番下（leaf）までkeyの位置に注意しながら降りていく
      inner = dynamic_cast<InnerNode *>(node);
      index = inner_position_for(key, inner->keys, inner->num_keys);
      node = inner->childref[index].get();
    }
    LeafNode *leaf = reinterpret_cast<LeafNode *>(node);
    index = leaf_position_for(key, leaf->keys, leaf->num_keys);
    if (leaf->keys[index] == key)
    {
      //もしkeyがあれば引数次第で値を代入
      if (value != 0)
      {
        *value = leaf->value[index];
      }
      return true;
    }
    else
      return false;
  }

  void delete_data(size_t &key)
  {
  }

  LeafNode select(size_t &minkey, size_t &maxkey)
  {
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
};