#include "../include/blocksRegistry.h"
#include <algorithm>
const unordered_map<string, int> BlockRegistry::blockTypes = {

    {"stone", 2},
    {"dirt", 3},
    {"grass_block", 4},
    {"oak_planks", 5},
    {"smooth_stone_slab", 6},
    {"smooth_stone", 7},
    {"bricks", 8},
    {"tnt", 9},
    {"water_bucket", 15},
    {"oak_sapling", 16},
    {"cobblestone", 17},
    {"bedrock", 18},
    {"oak_log", 21},
    {"iron_block", 23},
    {"gold_block", 24},
    {"diamond_block", 25},
    {"emerald_block", 26},
    {"redstone_block", 27},
    {"oak_leaves", 53},
    {"cyan_wool", 210},
    {"bookshelf", 36},
    {"oak_sign", 257},
    {"redstone", 258},
    {"compass", 259},
    {"diamond_pickaxe", 260},
    {"netherite_sword", 261},
    {"golden_apple", 262},
    {"iron_ingot", 263},
    {"creeper_spawn_egg", 264},
    {"chest", 265}

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
    {Category::All,
     []() {
         vector<int> all;
         all.reserve(blockTypes.size());
         for (const auto& pair : blockTypes) {
             all.push_back(pair.second);
         }
         sort(all.begin(), all.end());
         return all;
     }()}

};
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
