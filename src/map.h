#ifndef MAP_H
#define MAP_H

#include <vector>
#include "tile.h"

namespace Archipelago {

	extern const std::string& loggerName;
	class AssetRegistry;

	class Map {
	public:
		Map(AssetRegistry& assets) : _assets(assets), _areWaresVisible(false) {};
		void loadFromFile(const std::string& filename);
		unsigned int getTileWidth();
		unsigned int getTileHeight();
		unsigned int getMapWidth();
		unsigned int getMapHeight();
		const sf::Vector2f mapToScreenCoords(sf::Vector2f mapCoords);
		const sf::Vector2f screenToMapCoords(sf::Vector2f screenCoords);
		const sf::Vector2f getCenter();
		Archipelago::Tile* getTileAt(int mapX, int mapY);
		void setWaresVisibility(bool v) { _areWaresVisible = v; };
		bool getWaresVisibility() { return _areWaresVisible; };
		void draw(sf::RenderWindow& window);
	private:
		unsigned int _mapWidth;
		unsigned int _mapHeight;
		unsigned int _tileWidth;
		unsigned int _tileHeight;
		std::vector<Archipelago::Tile> _tiles;
		bool _areWaresVisible;
		AssetRegistry& _assets;
	};

}

#endif // MAP_H
