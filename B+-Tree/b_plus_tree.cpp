//reference -> https://en.wikibooks.org/wiki/Algorithm_Implementation/Trees/B%2B_tree
#include <iostream>
#include <vector>
#include <utility>
#include <memory>
#include <string>

const size_t maxLeafNum = 3;
const size_t maxInnerNum = 3;
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
  LeafNode() : NodeBase(maxLeafNum + 1), value(maxLeafNum + 1) {}
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
  InnerNode() : NodeBase(maxInnerNum + 1), childref(maxInnerNum + 2) {}
  ~InnerNode() {}
};

InnerNode *inner_cast(std::unique_ptr<NodeBase> &node)
{
  return dynamic_cast<InnerNode *>(node.get());
}

InnerNode *inner_cast(NodeBase *node)
{
  return dynamic_cast<InnerNode *>(node);
}

LeafNode *leaf_cast(std::unique_ptr<NodeBase> &node)
{
  return dynamic_cast<LeafNode *>(node.get());
}

LeafNode *leaf_cast(NodeBase *node)
{
  return dynamic_cast<LeafNode *>(node);
}

/*
* 挿入/削除した結果を格納する構造体(必要？)
*/
/*
struct InsertionResult
{
  size_t key;                      //insertしたkeyの値
  std::unique_ptr<NodeBase> left;  //左に何がいるのか
  std::unique_ptr<NodeBase> right; //右に何がいるのか
};
*/

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
  size_t node_position_for(const size_t &key, const std::vector<size_t> &keys, const size_t num_keys) const
  {
    size_t k = 0;
    while ((k < num_keys) && (keys[k] < key))
    {
      ++k;
    }
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
  /*
  void leaf_insert_nonfull(LeafNode *node, const size_t &key, const std::string &value, const size_t index)
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
  }*/

  /*
  * @breif 葉ノードに新しい値を挿入
  * @param [LeafNode&&]node 現在参照されている葉ノード
  * @param [const int&]key 挿入するキー
  * @param [const int&]numval 挿入する数値
  * @param [const string&]value 挿入する文字列
  * @param [InsertionResult*]result 分割した時の結果
  * @return [bool]was_split 挿入時分割したかどうか
  */
  /*bool leaf_insert(std::unique_ptr<NodeBase> &&node, const size_t &key, const std::string &value, InsertionResult &result)
  {
    bool was_split = false;
    size_t i = leaf_position_for(key, node->keys, node->num_keys);
    if (node->num_keys == maxLeafNum)
    {
      unsigned int threshold = minLeafNum;
      if (i < threshold)
        leaf_insert_nonfull(leaf_cast(node), key, value, i);
      else
        leaf_insert_nonfull(leaf_cast(new_sibling), key, value, i - threshold);
      //分割したこと（境界値、小さい方、大きい方）をお知らせする
      was_split = true;
      result.key = new_sibling->keys[0]; //分割した時の境界値
      result.left = std::move(node);
      result.right = std::move(new_sibling);
    }
    else //分割しなくてもいいので
      leaf_insert_nonfull(leaf_cast(node), key, value, i);
    return was_split;
  }*/

  /*
  * @brief 葉ノードにkey-value挿入
  * @param [LeafNode*]node
  * @param [size_t]key
  * @param [string]value
  */
  void leaf_insert(NodeBase *node, size_t &key, std::string value)
  {
    size_t index = node_position_for(key, node->keys, node->num_keys);
    if ((index < maxLeafNum) && (node->num_keys > 0) && (node->keys[index] == key))
    {
      leaf_cast(node)->value[index] = value;
    }
    else
    {
      for (size_t i = node->num_keys; i > index; --i)
      {
        node->keys[i] = node->keys[i - 1];
        leaf_cast(node)->value[i] = leaf_cast(node)->value[i - 1];
      }
      node->num_keys++;
      node->keys[index] = key;
      leaf_cast(node)->value[index] = value;
    }
  }

  /*
  * @breif 内部ノードが全て埋まっていない場合の内部ノードに新しい値を挿入
  * @param [InnerNode&&]node キーを挿入するところの内部ノード
  * @param [unsigned int]depth 内部ノードの深さ
  * @param [const int&]key 挿入するキー
  * @param [const int&]numval 挿入する数値
  * @param [const string&]value 挿入する文字列
  */
  /*
  void inner_insert_nonfull(NodeBase *node, unsigned int depth, const size_t &key, const std::string &value)
  {
    size_t index = inner_position_for(key, node->keys, node->num_keys);
    InsertionResult result;
    bool was_split;
    if (depth - 1 == 0)
    {
      //葉ノードに挿入
      was_split = leaf_insert(node, key, value, result);
      // node が空になるけど大丈夫？
    }
    else
    {
      //再帰的に内部ノードを走査する
      was_split = inner_insert(inner_cast(node)->childref[index], depth - 1, key, value, result);
      // childref[index] 空になるけど大丈夫？
    }
    if (was_split)
    {
      InnerNode *inode = inner_cast(node);
      if (index == node->num_keys)
      {
        // 一番右側に挿入する場合
        inode->keys[index] = result.key;
        inode->childref[index] = std::move(result.left);
        inode->childref[index + 1] = std::move(result.right);
        inode->num_keys++;
      }
      else
      {
        //間に入れるので、一個ずつ大きい方に移動してしてから挿入
        inode->childref[node->num_keys + 1] = std::move(inode->childref[node->num_keys]);
        for (unsigned i = node->num_keys; i != index; --i)
        {
          inode->childref[i] = std::move(inode->childref[i - 1]);
          inode->keys[i] = inode->keys[i - 1];
        }
        inode->childref[index] = std::move(result.left);
        inode->childref[index + 1] = std::move(result.right);
        inode->keys[index] = result.key;
        inode->num_keys++;
      }
    }
  }
  */

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
  /*
  bool inner_insert(NodeBase *node, unsigned int depth, const size_t &key, const std::string &value, InsertionResult &result)
  {
    //innerにkey,numval,valueを挿入してそっからleafまで辿って行って結果をresultに。その時nodeを分割したかどうか返す
    bool was_split = false;
    if (node->num_keys == maxInnerNum)
    {
      //分割したこと（境界値、その小さい方、大きい方）をお知らせする
      was_split = true;
      result.key = node->keys[minInnerNum - 1];
      result.left = std::move(node);
      result.right = std::move(split_inner(node));
      // 分割したどちらに新しい値を挿入するか
      if (key < (result.key))
        inner_insert_nonfull(node, depth, key, value);
      else
        inner_insert_nonfull(new_sibling, depth, key, value);
    }
    else //nodeの容量がまだいけるので
      inner_insert_nonfull(node, depth, key, value);
    return was_split;
  }
  */

  /*
  * @breif 内部ノードに挿入
  * @param [NodeBase*]node
  * @param [size_t]key
  * @param [unique_ptr<NodeBase>]small_node
  * @param [unique_ptr<NodeBase>]big_node
  * @param [size_t]index
  */
  void inner_insert(NodeBase *node, size_t &key, std::unique_ptr<NodeBase> &&inserted_node)
  {
    size_t index = node_position_for(key, node->keys, node->num_keys);
    if (index >= node->num_keys)
    {
      // 一番右側に挿入する場合
      inner_cast(node)->keys[index] = inserted_node->keys[0];
      inner_cast(node)->childref[index] = std::move(inserted_node);
    }
    else
    {
      //間に入れるので、一個ずつ大きい方に移動してしてから挿入
      for (unsigned int i = node->num_keys; i != index; --i)
      {
        inner_cast(node)->childref[i] = std::move(inner_cast(node)->childref[i - 1]);
        inner_cast(node)->keys[i] = inner_cast(node)->keys[i - 1];
      }
      inner_cast(node)->keys[index] = inserted_node->keys[0]; //エラー1
      inner_cast(node)->childref[index] = std::move(inserted_node);
    }
    inner_cast(node)->num_keys++;
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

  /*
  * @brief 求めたいkey-valueまでのrouteを見つける
  * @param [size_t&]key
  * @param [string&]value
  * @param [vector<NodeBase*>]routes
  * @return なし
  */
  void find(size_t &key, std::string &value, std::vector<NodeBase *> &routes)
  {
    InnerNode *inner;
    NodeBase *node;
    size_t index;
    for (unsigned int d = 1; d <= depth; ++d)
    {
      //一番下（leaf）までkeyの位置に注意しながら降りていく
      if (d == 1)
      {
        node = root.get();
      }
      else
      {
        inner = dynamic_cast<InnerNode *>(node);
        index = node_position_for(key, node->keys, node->num_keys);
        node = inner->childref[index].get();
      }
      routes.push_back(node);
    }
  }

  /*
  * @brief nodeを分割して、中身を整理
  * @param [NodeBase*]node
  * @return [unique_ptr<InnerNode>]new_sibling
  */
  std::unique_ptr<NodeBase> split_inner(NodeBase *node)
  {
    size_t threshold = (maxInnerNum - 1) / 2;
    //分割したnodeのindexが大きい方（兄）
    std::unique_ptr<NodeBase> new_sibling = std::make_unique<InnerNode>();
    new_sibling->num_keys = node->num_keys - threshold;
    for (unsigned int i = 0; i < new_sibling->num_keys; ++i)
    {
      //兄の方に入れる予定のkey, childrefを移し替え
      new_sibling->keys[i] = node->keys[threshold + i];
      inner_cast(new_sibling)->childref[i] = std::move(inner_cast(node)->childref[threshold + i]);
    }

    inner_cast(new_sibling)->childref[new_sibling->num_keys] = std::move(inner_cast(node)->childref[node->num_keys]);
    node->num_keys = threshold - 1; //分割されたnodeをindexが小さい方にする（弟）ので、要素数を変更
    new_sibling->num_keys--;
    return new_sibling;
  }

  /*
  * @brief nodeを分割して、中身を整理
  * @param [NodeBase*]node
  * @return [unique_ptr<LeafNode>]new_sibling
  */
  std::unique_ptr<NodeBase> split_leaf(NodeBase *node)
  {
    size_t threshold = (maxLeafNum - 1) / 2;
    //分割したnodeのindexが大きい方（兄）
    std::unique_ptr<NodeBase> new_sibling = std::make_unique<LeafNode>();
    new_sibling->num_keys = node->num_keys - threshold;
    for (unsigned int j = 0; j < new_sibling->num_keys; ++j)
    {
      //兄の方に入れる予定のkey, numval, valueを移し替え
      new_sibling->keys[j] = node->keys[threshold + j];
      leaf_cast(new_sibling)->value[j] = leaf_cast(node)->value[threshold + j];
    }
    node->num_keys = threshold;
    return new_sibling;
  }

  void lift_up(std::unique_ptr<NodeBase> &&right)
  {
    std::unique_ptr<NodeBase> old_root = std::move(root);
    root = std::make_unique<InnerNode>(); //rootを初期化
    root->keys[0] = right->keys[0];
    inner_cast(root)->childref[0] = std::move(old_root);
    inner_cast(root)->childref[1] = std::move(right);
    root->num_keys++;
    depth++;
  }

  void node_view(NodeBase *node, unsigned int d)
  {
    if (root->num_keys == 0)
    {
      std::cout << "None" << std::endl;
      return;
    }
    if (d == depth)
    {
      LeafNode *lnode = dynamic_cast<LeafNode *>(node);
      for (int i = 0; i < lnode->num_keys; ++i)
      {
        std::cout << "LEAF::depth: " << d << " key: " << lnode->keys[i] << " value: " << lnode->value[i] << std::endl;
      }
    }
    else
    {
      InnerNode *inode = dynamic_cast<InnerNode *>(node);
      for (int i = 0; i < inode->num_keys; ++i)
      {
        std::cout << "INNER::depth: " << d << " key: " << inode->keys[i] << std::endl;
        node_view(inode->childref[i].get(), d + 1);
      }
    }
    return;
  }

public:
  BPlusTree()
  {
    depth = 1;
    root = std::make_unique<LeafNode>();
  };
  ~BPlusTree(){};

  /*
  * @brief 新たな値を挿入/更新
  * @param [int]key
  * @param [int]numval
  * @param [string]value
  */

  void insert(size_t &key, std::string &value)
  {
    if (root->num_keys == 0)
    {
      leaf_insert(root.get(), key, value);
    }
    else
    {
      std::vector<NodeBase *> routes;
      find(key, value, routes);
      unsigned int d = routes.size();
      std::unique_ptr<NodeBase> new_sibling;
      for (int j = 0; j < routes.size(); ++j)
      {
        std::cout << typeid(routes[j]).name() << std::endl;
        for (int i = 0; i < routes[j]->num_keys; ++i)
        {
          std::cout << routes[j]->keys[i] << ' ';
        }
        std::cout << std::endl;
      }

      bool flag = false;
      leaf_insert(routes[d - 1], key, value);
      if (routes[d - 1]->num_keys >= maxLeafNum)
      {
        if (depth == 1)
          flag = true;
        new_sibling = split_leaf(routes[d - 1]);
      }
      d--;
      for (; d >= 1; --d)
      {
        inner_insert(routes[d - 1], key, std::move(new_sibling));
        if (routes[d - 1]->num_keys == maxInnerNum)
        {
          new_sibling = split_inner(routes[d - 1]);
          if (d == 1)
            flag = true;
        }
        else
        {
          break;
        }
      }
      if (new_sibling && depth > 1)
      {
        inner_insert(routes[d], key, std::move(new_sibling));
      }
      if (flag)
      {
        lift_up(std::move(new_sibling));
      }
    }
  }

  /*
  void insert(size_t key, std::string value)
  {
    InsertionResult result;
    bool was_split;
    if (depth == 0)
    {
      //rootがleafなら、そのままleafに挿入
      was_split = leaf_insert(std::move(root), key, value, result);
    }
    else
    {
      //rootがinnerなら、innerに代入して再帰的にchildrefを見て、innerになければ最終innerとleafに挿入。あればleafのみ更新する
      was_split = inner_insert(std::move(root), depth, key, value, result);
    }
    if (was_split)
    {
      //rootが分割されたら、新しくrootを作り直す
      depth++;                                 //深さが1つ増えるので
      root = std::make_unique<InnerNode>();    //rootを初期化
      InnerNode *rootProxy = inner_cast(root); //rootをポインタ（代理）から書き換える
      rootProxy->num_keys = 1;                 //現在新しく作られたところだから、要素は1つのみ
      //rootを分割するということは、resultで返ってきたやつは新しくrootに入るべきものなので
      rootProxy->keys[0] = result.key;
      rootProxy->childref[0] = std::move(result.left);
      rootProxy->childref[1] = std::move(result.right);
    }
  }
  */

  std::string find(const size_t &key)
  {
    if (root->num_keys == 0)
      return "";
    InnerNode *inner;
    NodeBase *node = root.get();
    for (unsigned int i = 0; i < depth - 1; ++i)
    {
      //一番下（leaf）までkeyの位置に注意しながら降りていく
      inner = dynamic_cast<InnerNode *>(node);
      size_t index = node_position_for(key, inner->keys, inner->num_keys);
      node = inner->childref[index].get();
    }
    LeafNode *target_node = dynamic_cast<LeafNode *>(node);
    size_t index = node_position_for(key, target_node->keys, target_node->num_keys);
    if (target_node->keys[index] == key)
    {
      return target_node->value[index];
    }
    else
      return "No key";
  }

  void view_root()
  {
    std::cout << "ROOT::" << std::endl;
    if (depth == 1)
    {
      for (int i = 0; i < root->num_keys; ++i)
      {
        std::cout << "key: " << root->keys[i] << " value: " << dynamic_cast<LeafNode *>(root.get())->value[i] << std::endl;
      }
    }
    else
    {
      for (int i = 0; i < root->num_keys; ++i)
      {
        std::cout << "key: " << root->keys[i] << std::endl;
      }
    }
  }

  void preview()
  {
    node_view(root.get(), 1);
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
