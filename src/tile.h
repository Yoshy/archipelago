#ifndef TILE_H
#define TILE_H

#include "SFML/Graphics.hpp"

namespace Archipelago {

	class Tile {
	public:
		Tile();
		~Tile();
	private:
		unsigned int _baseHeight;
		sf::Sprite _sprite;
	};

}

#endif // TILE_H
