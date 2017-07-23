#ifndef TILE_H
#define TILE_H

#include "SFML/Graphics.hpp"
#include <string>

namespace Archipelago {

	class Tile {
	public:
		Tile();
		~Tile();
		const std::string& getName();
		void setName(const std::string& tileName);
		void setTexture(const sf::Texture& texture);
		void setTileRising(unsigned int baseHeight);
		unsigned int getBaseHeight();
		sf::Sprite& getSprite();
		void setSpritePosition(float x, float y);
		void setSpritePosition(sf::Vector2f pos);
	private:
		std::string _name;
		unsigned int _rising;
		sf::Sprite _sprite;
	};

}

#endif // TILE_H
