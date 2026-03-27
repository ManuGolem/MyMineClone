#pragma once
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;
enum class Category {
    BuildingBlocks,
    ColoredBlocks,
    NaturalBlocks,
    FunctionalBlocks,
    RedstoneBlocks,
    SavedHotbars,
    ToolsUtilities,
    Combat,
    FoodDrinks,
    Ingredients,
    SpawnEggs,
    All
};
class BlockRegistry {
  private:
    static const unordered_map<string, int> blockTypes;
    static const unordered_map<Category, vector<int>> categories;

  public:
    static const vector<int>& get(Category category);
    static int getType(const string& blockName);
    static Category getCategory(int num);
};
