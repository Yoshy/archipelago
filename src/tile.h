#ifndef TILE_H
#define TILE_H

#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "wares.h"

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
		void addWares(WaresTypeId type, int amount);
		const std::vector<WaresStack>& getWaresStackList() { return _waresStackList; };
	private:
		std::string _name;
		unsigned int _rising;
		AssetRegistry& _assets;
		sf::Sprite _terrain_sprite;
		std::vector<WaresStack> _waresStackList;
	};

}

#endif // TILE_H
