#include "globals.h"
#include "map.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <map>
#include "json.hpp"

using namespace Archipelago;
using namespace std;
using namespace nlohmann;

Map::Map() {

}

Map::~Map() {

}

void Map::setTextureAtlas(TextureAtlas* atlasPtr) {
	_atlasPtr = atlasPtr;
}

void Map::loadFromFile(const std::string& filename) {
	auto logger = spdlog::get(LOGGER_NAME);

	if (!_atlasPtr) {
		logger->debug("Texture atlas is not set, can't load map. Set texture atlas before call to Map::loadFromFile");
		return;
	}
	logger->debug("Loading map '{}'", filename);
	json mapJSON;
	fstream mapFile;
	mapFile.open(filename);
	if (mapFile.fail()) {
		logger->debug("Map::loadFromFile failed. Error opening map file '{}'", filename);
		return;
	}
	mapFile >> mapJSON;
	mapFile.close();

	// Ошибки не проверяем, считаем, что json-файл карты корректный. Если упадём, то нечо лезть, куда не надо.
	_mapWidth = mapJSON.at("mapWidth");
	_mapHeight = mapJSON.at("mapHeight");
	_tileWidth = mapJSON.at("tileWidth");
	_tileHeight = mapJSON.at("tileHeight");
	unsigned int map_size = _mapWidth * _mapHeight;
	std::vector<unsigned int> map_data = mapJSON.at("map_data");
	if (map_data.size() != map_size) {
		logger->debug("map_data size ({}) is not equal to width*height ({}). There is something wrong in map file.", map_data.size(), map_size);
	}

	// Заполняем временный атлас тайлов, из которых состоит карта
	std::map<unsigned int, Tile> tileAtlas;
	for (json tileJSON : mapJSON.at("tileset")) {
		unsigned int id = tileJSON.at("id");
		std::string tileName = tileJSON.at("tileName");
		std::string texFileName = tileJSON.at("texFileName");
		unsigned int tileRising = tileJSON.find("tileRising") != tileJSON.end() ? tileJSON.at("tileRising") : 0;
		unsigned int texOffsetX = tileJSON.find("texOffsetX") != tileJSON.end() ? tileJSON.at("texOffsetX") : 0;
		_atlasPtr->loadFromFile(tileName, texFileName);
		Tile tile;
		tile.setName(tileName);
		tile.setTexture(_atlasPtr->getTexture(tileName));
		tile.getSprite().setTextureRect(sf::IntRect(texOffsetX, 0, _tileWidth, _atlasPtr->getTexture(tileName).getSize().y));
		tile.setTileRising(tileRising);
		tileAtlas[id] = tile;
	}
	// Формируем карту. Каждый тайл - уникальный объект, что позволит при необходимости кастомизировать свойства каждого отдельного тайла карты при необходимости.
	_tiles.reserve(map_size);
	float mapX = 0, mapY = 0;
	Tile tile;
	for (unsigned int tileID : map_data) {
		tile = tileAtlas[tileID];
		_tiles.push_back(tile);
		sf::Vector2f screenCoords = getScreenFromMapCoords(sf::Vector2f(mapX, mapY));
		screenCoords.y = screenCoords.y - tile.getBaseHeight();
		_tiles.back().setSpritePosition(screenCoords);
		mapX++;
		if (mapX >= _mapWidth) {
			mapX = 0;
			mapY++;
		}
	}
	logger->trace("Map width: {}, height: {}, _tiles size: {}", _mapWidth, _mapHeight, _tiles.size());
}

const sf::Vector2f Map::getScreenFromMapCoords(sf::Vector2f mapCoords) {
	auto logger = spdlog::get(LOGGER_NAME);
	sf::Vector2f screenCoords;
	screenCoords.x = (mapCoords.x * _tileWidth / 2) - (mapCoords.y * _tileWidth / 2) + 800;
	screenCoords.y = (mapCoords.x + mapCoords.y) * _tileHeight / 2;
	return screenCoords;
}

void Map::draw(sf::RenderWindow& window) {
	for (auto tile : _tiles) {
		sf::Sprite sprite = tile.getSprite();
		window.draw(sprite);
	}
}
