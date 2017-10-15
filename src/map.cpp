#include <fstream>
#include <map>
#include <spdlog/spdlog.h>
#include <json.hpp>
#include "map.h"
#include "asset_registry.h"

using namespace Archipelago;
using namespace nlohmann;

void Map::loadFromFile(const std::string& filename) {
	auto logger = spdlog::get(loggerName);

	logger->debug("Loading map '{}'", filename);
	json mapJSON;
	std::fstream mapFile;
	mapFile.open(filename);
	if (mapFile.fail()) {
		logger->debug("Map::loadFromFile failed. Error opening map file '{}'", filename);
		return;
	}
	mapFile >> mapJSON;
	mapFile.close();

	unsigned int map_size = 0;
	std::vector<unsigned int> terrain_layer;
	std::vector<unsigned int> wares_layer;
	try {
		_mapWidth = mapJSON.at("mapWidth");
		_mapHeight = mapJSON.at("mapHeight");
		_tileWidth = mapJSON.at("tileWidth");
		_tileHeight = mapJSON.at("tileHeight");
		map_size = _mapWidth * _mapHeight;
		terrain_layer = std::move(mapJSON.at("terrain_layer").get<std::vector<unsigned int>>());
		wares_layer = std::move(mapJSON.at("wares_layer").get<std::vector<unsigned int>>());
		if (terrain_layer.size() != map_size) {
			logger->debug("terrain_layer size ({}) is not equal to width*height ({}). There is something wrong in map file.", terrain_layer.size(), map_size);
		}
	}
	catch (std::out_of_range& e) {
		logger->error("Map::loadFromFile: Can't parse map file: {}", e.what());
	}

	// Заполняем временный атлас тайлов, из которых состоит карта
	std::map<unsigned int, Tile> tileAtlas;
	for (json tileJSON : mapJSON.at("tileset")) {
		unsigned int id = tileJSON.at("id");
		std::string tileName = tileJSON.at("tileName");
		std::string texFileName = tileJSON.at("texFileName");
		unsigned int tileRising = tileJSON.find("tileRising") != tileJSON.end() ? tileJSON.at("tileRising") : 0;
		unsigned int texOffsetX = tileJSON.find("texOffsetX") != tileJSON.end() ? tileJSON.at("texOffsetX") : 0;
		_assets.loadTexture(tileName, texFileName);
		tileAtlas.insert(std::pair<int, Tile>(id, std::move(Tile(_assets))));
		Tile& tile = tileAtlas.at(id);
		tile.setName(tileName);
		tile.setTexture(*_assets.getTexture(tileName));
		tile.getSprite().setTextureRect(sf::IntRect(texOffsetX, 0, _tileWidth, _assets.getTexture(tileName)->getSize().y));
		tile.setTileRising(tileRising);
	}
	// Формируем карту. Каждый тайл - уникальный объект, что позволит при необходимости кастомизировать свойства каждого отдельного тайла карты при необходимости.
	_tiles.reserve(map_size);
	double mapX = 0, mapY = 0;
	for (unsigned int i = 0; i < map_size; i++) {
		_tiles.push_back(tileAtlas.at(terrain_layer[i]));
		// Формат хранения материальных благ в файле: 32-битное число, каждый байт обозначает доступный на тайле тип ресурса, т.о.
		// на одном тайле может быть доступно до четырёх типов ресурсов.
		WaresTypeId waresType1 = static_cast<WaresTypeId>((wares_layer[i] & 0xFF000000) >> 24);
		WaresTypeId waresType2 = static_cast<WaresTypeId>((wares_layer[i] & 0x00FF0000) >> 16);
		WaresTypeId waresType3 = static_cast<WaresTypeId>((wares_layer[i] & 0x0000FF00) >> 8);
		WaresTypeId waresType4 = static_cast<WaresTypeId>((wares_layer[i] & 0x000000FF));

		WaresTypeId waresType;
		uint32_t bitMask = 0xFF000000;
		unsigned int shift = 24;
		for (unsigned int waresIdx = 0; waresIdx < 4; waresIdx++) {
			waresType = static_cast<WaresTypeId>((wares_layer[i] & bitMask) >> shift);
			shift -= 8;
			bitMask = bitMask >> 8;
			if (waresType != WaresTypeId::Unknown) {
				_tiles.back().addWares(waresType, 1);
			};
		}
		sf::Vector2f screenCoords = mapToScreenCoords(sf::Vector2f(static_cast<float>(mapX), static_cast<float>(mapY)));
		screenCoords.y = screenCoords.y - _tiles.back().getTileRising();
		_tiles.back().setSpritePosition(screenCoords);
		mapX++;
		if (mapX >= _mapWidth) {
			mapX = 0;
			mapY++;
		}
	}
	logger->trace("Map width: {}, height: {}, _tiles size: {}", _mapWidth, _mapHeight, _tiles.size());
}

unsigned int Map::getTileWidth() {
	return _tileWidth;
}

unsigned int Map::getTileHeight() {
	return _tileHeight;
}

unsigned int Map::getMapWidth() {
	return _mapWidth;
}

unsigned int Map::getMapHeight() {
	return _mapHeight;
}

const sf::Vector2f Map::mapToScreenCoords(sf::Vector2f mapCoords) {
	sf::Vector2f screenCoords;
	screenCoords.x = (mapCoords.x - mapCoords.y) * _tileWidth / 2;
	screenCoords.y = (mapCoords.x + mapCoords.y) * _tileHeight / 2;
	return screenCoords;
}

const sf::Vector2f Map::screenToMapCoords(sf::Vector2f screenCoords) {
	sf::Vector2f mapCoords;
	mapCoords.x = (screenCoords.x / (_tileWidth / 2) + screenCoords.y / (_tileHeight / 2)) / 2 - 0.5f;
	mapCoords.y = (screenCoords.y / (_tileHeight / 2) - (screenCoords.x / (_tileWidth / 2))) / 2 + 0.5f;
	return sf::Vector2f(mapCoords);
}

const sf::Vector2f Map::getCenter() {
	sf::Vector2f center;
	center.x = static_cast<float>(_tileWidth) / 2;
	center.y = static_cast<float>(_tileHeight * _mapHeight) / 2;
	return center;
}

Archipelago::Tile* Map::getTileAt(int mapX, int mapY) {
	auto logger = spdlog::get(loggerName);
	int tileIdx = mapY * _mapWidth + mapX;
	if (mapX < 0 || mapX >= static_cast<int>(_mapWidth) || mapY < 0 || mapY >= static_cast<int>(_mapHeight)) {
		return nullptr;
	}
	if (tileIdx >= 0 && tileIdx < static_cast<int>(_tiles.size())) {
		return &_tiles[tileIdx];
	}
	return nullptr;
}

void Map::draw(sf::RenderWindow& window) {
	for (auto& tile : _tiles) {
		sf::Sprite tileSprite = tile.getSprite();
		window.draw(tileSprite);
		if (_areWaresVisible) {
			auto waresStack = tile.getWaresStackList();
			unsigned int numWares = waresStack.size();
			// На тайл влезет примерно (tileWidth - iconWidth) * 2 иконок товаров
			for (unsigned int g = 0; g < numWares; g++) {
				sf::Sprite waresSprite;
				waresSprite.setTexture(*_assets.getWaresSpecification(waresStack[g].type).icon);
				auto gsTexSize = waresSprite.getTexture()->getSize();
				auto waresSpritePos = tileSprite.getPosition();
				waresSpritePos.x += (_tileWidth / 2) + ((gsTexSize.x) * (g - (numWares / 2)));
				waresSpritePos.y += (_tileHeight / 2) - (gsTexSize.y / 2);
				waresSprite.setPosition(waresSpritePos);
				window.draw(waresSprite);
			}
		}
	}
}
