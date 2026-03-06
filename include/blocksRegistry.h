#pragma once
#include <unordered_map>
#include <vector>

enum class Category { BuildingBlocks, ColoredBlocks, NaturalBlocks, FunctionalBlocks, RedstoneBlocks, ToolsUtilities, Combat, FoodDrinks, Ingredients, SpawnEggs, All };
class BlockRegistry {
  public:
    inline static const std::unordered_map<Category, std::vector<int>> categories = {

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
    static const std::vector<int>& get(Category category) {
        return categories.at(category);
    }
};
