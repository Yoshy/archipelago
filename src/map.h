#ifndef MAP_H
#define MAP_H

#include <vector>
#include "tile.h"
#include "texture_atlas.h"

namespace Archipelago {

	class Map {
	public:
		Map();
		~Map();
		void setTextureAtlas(TextureAtlas* atlasPtr);
		void loadFromFile(const std::string& filename);
		const sf::Vector2f getScreenFromMapCoords(sf::Vector2f mapCoords);
		void draw(sf::RenderWindow& window);
	private:
		unsigned int _mapWidth;
		unsigned int _mapHeight;
		unsigned int _tileWidth;
		unsigned int _tileHeight;
		std::vector<Archipelago::Tile> _tiles;
		TextureAtlas* _atlasPtr;
	};

}

#endif // MAP_H
