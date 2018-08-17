//reference -> https://en.wikibooks.org/wiki/Algorithm_Implementation/Trees/B%2B_tree
#include <iostream>
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <cassert>

const size_t maxLeafNum = 3;
const size_t maxInnerNum = 3;
const size_t minLeafNum = (maxLeafNum + 1) / 2;
const size_t minInnerNum = (maxInnerNum - 1) / 2;

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
  virtual bool isLeaf() const {};
  virtual ~NodeBase() {}
};

/*
* 葉ノードの構造体
* @param maxLeafNum 要素数確保
* @detail [string]value 文字列配列
*/
struct LeafNode : NodeBase
{
  std::vector<std::string> values;
  LeafNode() : NodeBase(maxLeafNum + 1), values(maxLeafNum + 1) {}
  bool isLeaf() const
  {
    return true;
  }
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
  bool isLeaf() const
  {
    return false;
  }
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
      leaf_cast(node)->values[index] = value;
    }
    else
    {
      for (size_t i = node->num_keys; i > index; --i)
      {
        node->keys[i] = node->keys[i - 1];
        leaf_cast(node)->values[i] = leaf_cast(node)->values[i - 1];
      }
      node->num_keys++;
      node->keys[index] = key;
      leaf_cast(node)->values[index] = value;
    }
  }

  /*
  * @breif 内部ノードに挿入
  * @param [NodeBase*]node
  * @param [size_t]key
  * @param [unique_ptr<NodeBase>]small_node
  * @param [unique_ptr<NodeBase>]big_node
  * @param [size_t]index
  */
  void inner_insert(NodeBase *node, size_t &key, std::unique_ptr<NodeBase> &&inserted_node, size_t inserted_key)
  {
    size_t index = node_position_for(key, node->keys, node->num_keys);
    if (index > node->num_keys)
    {
      // 一番右側に挿入する場合
      inner_cast(node)->keys[index] = inserted_key;
      inner_cast(node)->childref[index + 1] = std::move(inserted_node);
    }
    else
    {
      //間に入れるので、一個ずつ大きい方に移動してしてから挿入
      for (unsigned int i = node->num_keys; i != index; --i)
      {
        inner_cast(node)->childref[i + 1] = std::move(inner_cast(node)->childref[i]);
        inner_cast(node)->keys[i] = inner_cast(node)->keys[i - 1];
      }
      inner_cast(node)->keys[index] = inserted_key;
      inner_cast(node)->childref[index + 1] = std::move(inserted_node);
    }
    inner_cast(node)->num_keys++;
  }

  /*
  * @breif 既にある葉ノードをupdate
  * @param [NodeBase*]node
  * @param [size_t]key
  * @param [string]value
  */
  void leaf_update(NodeBase *node, size_t &key, std::string value)
  {
    LeafNode *lnode = dynamic_cast<LeafNode *>(node);
    size_t index = node_position_for(key, lnode->keys, lnode->num_keys);
    lnode->values[index] = value;
  }

  /*
  * @brief 葉ノード内の対象のkey-valueをdelete
  * @param [NodeBase*]node
  * @param [size_t]key
  */
  void leaf_delete(NodeBase *node, size_t &key)
  {
    LeafNode *lnode = dynamic_cast<LeafNode *>(node);
    size_t index = node_position_for(key, lnode->keys, lnode->num_keys);
    for (size_t i = index; i < lnode->num_keys - 1; ++i)
    {
      lnode->keys[i] = lnode->keys[i + 1];
      lnode->values[i] = lnode->values[i + 1];
    }
    lnode->num_keys--;
  }

  /*
  * @brief 内部ノード内の対象のkey-valueをdelete
  * @param [NodeBase*]node
  * @param [size_t]key
  */
  void inner_delete(NodeBase *node, size_t &key)
  {
    InnerNode *inode = dynamic_cast<InnerNode *>(node);
    size_t index = node_position_for(key, inode->keys, inode->num_keys);
    if (inode->keys[index] == key)
      index++;
    printf("inner index:%zu\n", index);
    if (inode->childref[index]->isLeaf() && inode->childref[index]->keys[0] == inode->keys[index - 1])
    {
      inode->keys[index - 1] = inode->childref[index + 1]->keys[1];
    }
    else if (!inode->childref[index]->isLeaf() && inode->childref[index]->keys[0] != inode->keys[index - 1])
    {
      inode->keys[index - 1] = inode->childref[index]->keys[0];
    }
    else
    {
      for (size_t i = index; i < inode->num_keys - 1; ++i)
      {
        inode->keys[i] = inode->keys[i + 1];
      }
      inode->num_keys--;
    }
  }

  /*
  * @brief 空になったノードをmergeする
  * @param [NodeBase*]parent
  * @param [NodeBase*]child
  */
  void
  merge(NodeBase *parent, size_t &child_key)
  {
    size_t index = node_position_for(child_key, parent->keys, parent->num_keys);
    InnerNode *iparent = dynamic_cast<InnerNode *>(parent);
    for (size_t i = index; i < parent->num_keys; ++i)
    {
      iparent->childref[i] = std::move(iparent->childref[i + 1]);
    }
  }

  /*
  * @brief 求めたいkey-valueまでのrouteを見つける
  * @param [size_t&]key
  * @param [string&]value
  * @param [vector<NodeBase*>]routes
  * @return なし
  */
  bool find(const size_t &key, const std::string &value, std::vector<NodeBase *> &routes)
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
    LeafNode *lnode = dynamic_cast<LeafNode *>(node);
    index = node_position_for(key, lnode->keys, lnode->num_keys);
    if (key == lnode->keys[index] && value == lnode->values[index])
      return true;
    else
      return false;
  }

  /*
  * @brief 求めたいkey-valueまでのrouteを見つける
  * @param [size_t&]key
  * @param [vector<NodeBase*>]routes
  */
  bool find(const size_t &key, std::vector<NodeBase *> &routes)
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
        if (inner->keys[index] == key)
          index++;
        printf("inner find depth:%zu, index:%zu\n", d, index);
        node = inner->childref[index].get();
      }
      routes.push_back(node);
    }
    LeafNode *lnode = dynamic_cast<LeafNode *>(node);
    index = node_position_for(key, lnode->keys, lnode->num_keys);
    if (key == lnode->keys[index])
      return true;
    else
      return false;
  }

  /*
  * @brief nodeを分割して、中身を整理
  * @param [NodeBase*]node
  * @return [unique_ptr<InnerNode>]new_sibling
  */
  std::unique_ptr<NodeBase> split_inner(NodeBase *node, size_t &key)
  {
    size_t threshold = maxInnerNum / 2;
    //分割したnodeのindexが大きい方（兄）
    std::unique_ptr<NodeBase> new_sibling = std::make_unique<InnerNode>();
    new_sibling->num_keys = node->num_keys - threshold;
    for (unsigned int i = 0; i < new_sibling->num_keys - 1; ++i)
    {
      //兄の方に入れる予定のkey, childrefを移し替え
      new_sibling->keys[i] = node->keys[threshold + i + 1];
      inner_cast(new_sibling)->childref[i] = std::move(inner_cast(node)->childref[threshold + i + 1]);
    }
    inner_cast(new_sibling)->childref[new_sibling->num_keys - 1] = std::move(inner_cast(node)->childref[threshold + new_sibling->num_keys]);
    key = node->keys[threshold];

    node->num_keys = threshold; //分割されたnodeはindexが小さい方になる（弟）ので、要素数を変更
    new_sibling->num_keys--;
    return new_sibling;
  }

  /*
  * @brief nodeを分割して、中身を整理
  * @param [NodeBase*]node
  * @return [unique_ptr<LeafNode>]new_sibling
  */
  std::unique_ptr<NodeBase> split_leaf(NodeBase *node, size_t &key)
  {
    size_t threshold = maxLeafNum / 2;
    //分割したnodeのindexが大きい方（兄）
    std::unique_ptr<NodeBase> new_sibling = std::make_unique<LeafNode>();
    new_sibling->num_keys = node->num_keys - threshold;
    for (unsigned int j = 0; j < new_sibling->num_keys; ++j)
    {
      //兄の方に入れる予定のkey, numval, valueを移し替え
      new_sibling->keys[j] = node->keys[threshold + j];
      leaf_cast(new_sibling)->values[j] = leaf_cast(node)->values[threshold + j];
    }
    key = new_sibling->keys[0];
    node->num_keys = threshold;
    return new_sibling;
  }

  void lift_up(std::unique_ptr<NodeBase> &&right, size_t &key)
  {
    std::unique_ptr<NodeBase> old_root = std::move(root);
    root = std::make_unique<InnerNode>(); //rootを初期化
    root->keys[0] = key;

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
    if (node->isLeaf())
    {
      LeafNode *lnode = dynamic_cast<LeafNode *>(node);
      for (int i = 0; i < lnode->num_keys; ++i)
      {
        std::cout << "LEAF::depth: " << d << " key: " << lnode->keys[i] << " value: " << lnode->values[i] << std::endl;
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
      node_view(inode->childref[inode->num_keys].get(), d + 1);
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

  void insert(size_t key, std::string value)
  {
    if (root->num_keys == 0)
    {
      leaf_insert(root.get(), key, value);
    }
    else
    {
      std::vector<NodeBase *> routes;
      bool existPair = find(key, value, routes);
      if (existPair)
      {
        printf("The key %zu and value %s already exist.\n", key, value.c_str());
        return;
      }
      unsigned int d = routes.size();
      std::unique_ptr<NodeBase> new_sibling;

      bool flag = false;
      leaf_insert(routes[d - 1], key, value);
      size_t splited_key;
      if (routes[d - 1]->num_keys >= maxLeafNum)
      {
        if (depth == 1)
          flag = true;
        new_sibling = split_leaf(routes[d - 1], splited_key);
      }
      else
      {
        printf("Insert key-value %zu-%s successfully!\n", key, value.c_str());
        return;
      }
      d--;
      for (; d >= 1; --d)
      {
        inner_insert(routes[d - 1], key, std::move(new_sibling), splited_key);
        if (routes[d - 1]->num_keys == maxInnerNum)
        {
          new_sibling = split_inner(routes[d - 1], splited_key);
          if (d == 1)
          {
            flag = true;
          }
        }
        else
        {
          break;
        }
      }
      if (new_sibling && depth > 1 && d != 0)
      {
        inner_insert(routes[d], key, std::move(new_sibling), splited_key);
      }
      else if (flag)
      {
        lift_up(std::move(new_sibling), splited_key);
      }
      printf("Insert key-value %zu-%s successfully!\n", key, value.c_str());
    }
  }

  void update(size_t key, std::string value)
  {
    if (root->num_keys == 0)
    {
      printf("There are no key-value pairs.\n", key, value);
      return;
    }
    std::vector<NodeBase *> routes;
    bool existValue = find(key, routes);
    if (existValue)
    {
      leaf_update(routes[routes.size() - 1], key, value);
    }
    else
    {
      printf("This key %zu is nil.\n");
      return;
    }
  }

  /*
  * @breif あるkey-valueをdeleteする
  * @param [size_t]key
  */
  void delete_(size_t key)
  {
    if (root->num_keys == 0)
    {
      printf("There are no key-value pairs.\n");
      return;
    }
    std::vector<NodeBase *> routes;
    bool existValue = find(key, routes);
    if (existValue)
    {
      for (unsigned int d = depth; d >= 1; --d)
      {
        if (d == depth)
        {
          leaf_delete(routes[d - 1], key);
          if (routes[d - 1]->num_keys <= 0 && d >= 2)
          {
            merge(routes[d - 2], key);
          }
        }

        else
        {
          inner_delete(routes[d - 1], key);
          if (routes[d - 1]->num_keys < 0 && d >= 2)
          {
            merge(routes[d - 2], key);
          }
        }
        //preview();
        printf("\n");
      }
      if (root->num_keys <= 0 && depth > 1)
      {
        root = std::move(dynamic_cast<InnerNode *>(root.get())->childref[0]);
      }
      printf("Delete key %zu successfully!\n", key);
    }
    else
    {
      printf("This key %zu is nil.\n", key);
      return;
    }
  }

  void find(const size_t &key)
  {
    if (root->num_keys == 0)
      printf("\n");
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
      printf("value: %s\n", target_node->values[index].c_str());
    }
    else
      printf("No key\n");
  }

  void view_root()
  {
    std::cout << "ROOT::" << std::endl;
    if (depth == 1)
    {
      for (int i = 0; i < root->num_keys; ++i)
      {
        std::cout << "key: " << root->keys[i] << " value: " << dynamic_cast<LeafNode *>(root.get())->values[i] << std::endl;
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
