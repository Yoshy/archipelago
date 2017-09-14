#include <fstream>
#include <map>
#include "globals.h"
#include "spdlog/spdlog.h"
#include "json.hpp"
#include "map.h"
#include "asset_registry.h"

using namespace Archipelago;
using namespace std;
using namespace nlohmann;

void Map::loadFromFile(const std::string& filename) {
	auto logger = spdlog::get(LOGGER_NAME);

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
	std::vector<unsigned int> terrain_layer = mapJSON.at("terrain_layer");
	std::vector<unsigned int> goods_layer = mapJSON.at("goods_layer");
	if (terrain_layer.size() != map_size) {
		logger->debug("terrain_layer size ({}) is not equal to width*height ({}). There is something wrong in map file.", terrain_layer.size(), map_size);
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
		GoodsTypeId goodsType1 = static_cast<GoodsTypeId>((goods_layer[i] & 0xFF000000) >> 24);
		GoodsTypeId goodsType2 = static_cast<GoodsTypeId>((goods_layer[i] & 0x00FF0000) >> 16);
		GoodsTypeId goodsType3 = static_cast<GoodsTypeId>((goods_layer[i] & 0x0000FF00) >> 8);
		GoodsTypeId goodsType4 = static_cast<GoodsTypeId>((goods_layer[i] & 0x000000FF));

		GoodsTypeId goodsType;
		uint32_t bitMask = 0xFF000000;
		unsigned int shift = 24;
		for (unsigned int goodsIdx = 0; goodsIdx < 4; goodsIdx++) {
			goodsType = static_cast<GoodsTypeId>((goods_layer[i] & bitMask) >> shift);
			shift -= 8;
			bitMask = bitMask >> 8;
			if (goodsType != GoodsTypeId::Unknown) {
				_tiles.back().addGoods(goodsType, 1);
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
	auto logger = spdlog::get(LOGGER_NAME);
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
		if (_isGoodsVisible) {
			auto goodsStack = tile.getGoodsStackList();
			unsigned int numGoods = goodsStack.size();
			// На тайл влезет примерно (tileWidth - iconWidth) * 2 иконок товаров
			for (unsigned int g = 0; g < numGoods; g++) {
				sf::Sprite goodsSprite;
				goodsSprite.setTexture(*_assets.getGoodsSpecification(goodsStack[g].type).icon);
				auto gsTexSize = goodsSprite.getTexture()->getSize();
				auto goodsSpritePos = tileSprite.getPosition();
				goodsSpritePos.x += (_tileWidth / 2) + ((gsTexSize.x) * (g - (numGoods / 2)));
				goodsSpritePos.y += (_tileHeight / 2) - (gsTexSize.y / 2);
				goodsSprite.setPosition(goodsSpritePos);
				window.draw(goodsSprite);
			}
		}
	}
}
