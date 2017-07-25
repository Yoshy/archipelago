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
		unsigned int getTileWidth();
		unsigned int getTileHeight();
		unsigned int getMapWidth();
		unsigned int getMapHeight();
		const sf::Vector2f mapToScreenCoords(sf::Vector2f mapCoords);
		const sf::Vector2f screenToMapCoords(sf::Vector2f screenCoords);
		const sf::Vector2f getCenter();
		Archipelago::Tile* getTileAt(int mapX, int mapY);
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
