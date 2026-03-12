#include "../include/blocksRegistry.h"
const unordered_map<string, int> BlockRegistry::blockTypes = {
    {"stone", 2},
    {"dirt", 3},
    {"grass_block", 4},
};

const unordered_map<Category, std::vector<int>> BlockRegistry::categories = {
    {Category::BuildingBlocks, {8}},
    {Category::ColoredBlocks, {210}},
    {Category::NaturalBlocks, {4}},
    {Category::FunctionalBlocks, {257}},
    {Category::RedstoneBlocks, {258}},
    {Category::ToolsUtilities, {260}},
    {Category::Combat, {261}},
    {Category::FoodDrinks, {262}},
    {Category::Ingredients, {263}},
    {Category::SpawnEggs, {264}},
    {Category::All, {2, 3, 4, 5, 6, 7, 8, 17, 36, 210, 257, 258, 259, 260, 261, 262, 263, 264, 265}}};

const vector<int>& BlockRegistry::get(Category category) {
    return categories.at(category);
}

int BlockRegistry::getType(const string& blockName) {
    auto it = blockTypes.find(blockName);
    if (it != blockTypes.end()) {
        return it->second;
    }
    return -1;
}

Category BlockRegistry::getCategory(int num) {
    if (num == 1)
        return Category::BuildingBlocks;
    if (num == 2)
        return Category::ColoredBlocks;
    if (num == 3)
        return Category::NaturalBlocks;
    if (num == 4)
        return Category::FunctionalBlocks;
    if (num == 5)
        return Category::RedstoneBlocks;
    if (num == 7)
        return Category::All;
    if (num == 8)
        return Category::ToolsUtilities;
    if (num == 9)
        return Category::Combat;
    if (num == 10)
        return Category::FoodDrinks;
    if (num == 11)
        return Category::Ingredients;
    if (num == 12)
        return Category::SpawnEggs;
    return Category::All;
}
