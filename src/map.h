#ifndef MAP_H
#define MAP_H

#include "tile.h"
#include <vector>

namespace Archipelago {

	class Map {
	public:
		Map();
		void draw(const sf::RenderWindow& window);
	private:
		unsigned int _width;
		unsigned int _height;
		std::vector<Archipelago::Tile> _tiles;
	};

}

#endif // MAP_H
