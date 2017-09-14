#ifndef TILE_H
#define TILE_H

#include "SFML/Graphics.hpp"
#include <string>
#include "goods.h"

namespace Archipelago {

	class AssetRegistry;

	class Tile {
	public:
		Tile(AssetRegistry& assets);
		const std::string& getName();
		void setName(const std::string& tileName);
		void setTexture(const sf::Texture& texture);
		void setTileRising(unsigned int rising);
		unsigned int getTileRising();
		sf::Sprite& getSprite();
		void setSpritePosition(float x, float y);
		void setSpritePosition(sf::Vector2f pos);
		void addGoods(GoodsTypeId type, int amount);
		const std::vector<GoodsStack>& getGoodsStackList() { return _goodsStackList; };
	private:
		std::string _name;
		unsigned int _rising;
		AssetRegistry& _assets;
		sf::Sprite _terrain_sprite;
		std::vector<GoodsStack> _goodsStackList;
	};

}

#endif // TILE_H
