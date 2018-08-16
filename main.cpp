#include "B+-Tree/b_plus_tree.cpp"

int main()
{
  BPlusTree bt;
  std::string op;
  /*while (true)
  {
    std::cin >> op;
    if (op == "insert")
    {
      size_t num;
      std::string str;
      std::cin >> num >> str;
      bt.insert(num, str);
    }
    else if (op == "find")
    {
      int num;
      std::cin >> num;
      std::string value = bt.find(num);
      if (!value.empty())
        std::cout << "value: " << value << std::endl;
      else
        std::cout << "There are no values" << std::endl;
    }
    else if (op == "view")
      bt.preview();
    else if (op == "root")
      bt.view_root();
    else if (op == "end")
      break;
    else
    {
      std::cout << "Invalid input" << std::endl;
      continue;
    
  }*/
  std::vector<size_t> num = {1, 2, 3, 4, 5};
  std::vector<std::string> str = {"a", "b", "c", "d", "e"};
  for (int i = 0; i < 5; ++i)
  {
    bt.insert(num[i], str[i]);
  }

  return 0;
}