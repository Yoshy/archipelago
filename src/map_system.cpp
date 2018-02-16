#include <spdlog\spdlog.h>
#include <fstream>
#include <json.hpp>
#include "map_system.h"

using namespace Archipelago;

namespace Archipelago {
	const float maxCameraZoom{ 3.0f };
	const float minCameraZoom{ 0.2f };
}

void MapSystem::configure(World* world) {
	spdlog::get(loggerName)->trace("MapSystem::configure started");
	world->subscribe<LoadMapEvent>(this);
	world->subscribe<MoveCameraEvent>(this);
	world->subscribe<MoveCameraToMapCenterEvent>(this);
	world->subscribe<ZoomCameraEvent>(this);
	world->subscribe<ConvertScreenToMapCoordsEvent>(this);
	world->subscribe<ConvertMapToScreenCoordsEvent>(this);
	_curCameraZoom = 1.0f;
}

void MapSystem::unconfigure(World* world) {
	spdlog::get(loggerName)->trace("MapSystem::unconfigure started");
	world->unsubscribe<LoadMapEvent>(this);
	world->unsubscribe<MoveCameraEvent>(this);
	world->unsubscribe<MoveCameraToMapCenterEvent>(this);
	world->unsubscribe<ZoomCameraEvent>(this);
	world->unsubscribe<ConvertScreenToMapCoordsEvent>(this);
	world->unsubscribe<ConvertMapToScreenCoordsEvent>(this);
}

void MapSystem::receive(World* world, const LoadMapEvent& event) {
	auto logger = spdlog::get(loggerName);
	logger->trace("MapSystem: LoadMapEvent received. Loading map '{}'", event.filename);
	nlohmann::json mapJSON;
	std::fstream mapFile;
	mapFile.open(event.filename);
	if (mapFile.fail()) {
		logger->error("MapSystem: Error opening map file '{}'", event.filename);
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
			logger->error("terrain_layer size ({}) is not equal to width*height ({}). There is something wrong in map file.", terrain_layer.size(), map_size);
		}
	}
	catch (std::out_of_range& e) {
		logger->error("MapSystem: Can't parse map file: {}", e.what());
	}

	// Заполняем временный атлас тайлов, из которых состоит карта
	std::map<unsigned int, TileComponent> tileAtlas;
	for (nlohmann::json tileJSON : mapJSON.at("tileset")) {
		unsigned int id = tileJSON.at("id");
		std::string tileName = tileJSON.at("tileName");
		std::string texFileName = tileJSON.at("texFileName");
		unsigned int tileRising = tileJSON.find("tileRising") != tileJSON.end() ? tileJSON.at("tileRising") : 0;
		unsigned int texOffsetX = tileJSON.find("texOffsetX") != tileJSON.end() ? tileJSON.at("texOffsetX") : 0;
		_game.getAssetRegistry().loadTexture(tileName, texFileName);
		tileAtlas.insert(std::pair<int, TileComponent>(id, std::move(TileComponent())));
		TileComponent& tileComponent = tileAtlas.at(id);
		tileComponent.sprite.setTexture(*_game.getAssetRegistry().getTexture(tileName));
		tileComponent.sprite.setTextureRect(sf::IntRect(texOffsetX, 0, _tileWidth, _game.getAssetRegistry().getTexture(tileName)->getSize().y));
		tileComponent.rising = tileRising;
	}
	// Формируем карту. Каждый тайл - уникальный объект, что позволит при необходимости кастомизировать свойства каждого отдельного тайла карты при необходимости.
	unsigned int mapX = 0, mapY = 0;
	TileComponent tmpTileComponent;
	for (unsigned int i = 0; i < map_size; i++) {
		tmpTileComponent = tileAtlas.at(terrain_layer[i]);
		// Формат хранения материальных благ в файле: 32-битное число, каждый байт обозначает доступный на тайле тип ресурса, т.о.
		// на одном тайле может быть доступно до четырёх типов ресурсов.
		//WaresTypeId waresType1 = static_cast<WaresTypeId>((wares_layer[i] & 0xFF000000) >> 24);
		//WaresTypeId waresType2 = static_cast<WaresTypeId>((wares_layer[i] & 0x00FF0000) >> 16);
		//WaresTypeId waresType3 = static_cast<WaresTypeId>((wares_layer[i] & 0x0000FF00) >> 8);
		//WaresTypeId waresType4 = static_cast<WaresTypeId>((wares_layer[i] & 0x000000FF));

		//WaresTypeId waresType;
		//uint32_t bitMask = 0xFF000000;
		//unsigned int shift = 24;
		//for (unsigned int waresIdx = 0; waresIdx < 4; waresIdx++) {
		//	waresType = static_cast<WaresTypeId>((wares_layer[i] & bitMask) >> shift);
		//	shift -= 8;
		//	bitMask = bitMask >> 8;
		//	if (waresType != WaresTypeId::Unknown) {
		//		_tiles.back().addWares(waresType, 1);
		//	};
		//}
		sf::Vector2f screenCoords = _mapToScreenCoords(sf::Vector2f(static_cast<float>(mapX), static_cast<float>(mapY)));
		screenCoords.y = screenCoords.y - tmpTileComponent.rising;
		tmpTileComponent.sprite.setPosition(screenCoords);
		Entity* ent = world->create();
		ent->assign<TileComponent>(tmpTileComponent.rising, tmpTileComponent.sprite, mapX, mapY);
		mapX++;
		if (mapX >= _mapWidth) {
			mapX = 0;
			mapY++;
		}
	}
	logger->trace("MapSystem: Map loaded. Width: {}, height: {}", _mapWidth, _mapHeight);
}

void MapSystem::receive(World* world, const MoveCameraEvent& event) {
	//spdlog::get(loggerName)->trace("MoveCameraEvent received. Offset ({}, {})", event.offsetX, event.offsetY);
	sf::View v = _game.getRenderWindow().getView();
	sf::Vector2f viewCenter = v.getCenter();
	viewCenter.x = viewCenter.x + event.offsetX;
	viewCenter.y = viewCenter.y + event.offsetY;
	sf::Vector2f mapCoords = _screenToMapCoords(viewCenter);
	if (mapCoords.x < 0 || mapCoords.y < 0 || mapCoords.x > _mapWidth || mapCoords.y > _mapHeight) {
		return;
	}
	v.move(event.offsetX, event.offsetY);
	_game.getRenderWindow().setView(v);
}

void MapSystem::receive(World* world, const MoveCameraToMapCenterEvent& event) {
	sf::View view = _game.getRenderWindow().getView();
	view.setCenter(static_cast<float>(_tileWidth) / 2, static_cast<float>(_tileHeight * _mapHeight) / 2);
	_game.getRenderWindow().setView(view);
}

void MapSystem::receive(World* world, const ZoomCameraEvent& event) {
	//spdlog::get(loggerName)->trace("MapSystem: ZoomCameraEvent received. Zoom factor {}", event.zoomFactor);
	if (_curCameraZoom * event.zoomFactor > maxCameraZoom || _curCameraZoom * event.zoomFactor < minCameraZoom) {
		return;
	}
	sf::View v = _game.getRenderWindow().getView();
	v.zoom(event.zoomFactor);
	_game.getRenderWindow().setView(v);
	_curCameraZoom *= event.zoomFactor;
}

void MapSystem::receive(World* world, const ConvertScreenToMapCoordsEvent& event) {
	event.coords = _screenToMapCoords(event.coords);
}

void MapSystem::receive(World* world, const ConvertMapToScreenCoordsEvent& event) {
	event.coords = _mapToScreenCoords(event.coords);
}

const sf::Vector2f MapSystem::_mapToScreenCoords(sf::Vector2f mapCoords) {
	sf::Vector2f screenCoords;
	screenCoords.x = (mapCoords.x - mapCoords.y) * _tileWidth / 2;
	screenCoords.y = (mapCoords.x + mapCoords.y) * _tileHeight / 2;
	return screenCoords;
}

const sf::Vector2f MapSystem::_screenToMapCoords(sf::Vector2f screenCoords) {
	sf::Vector2f mapCoords;
	mapCoords.x = (screenCoords.x / (_tileWidth / 2) + screenCoords.y / (_tileHeight / 2)) / 2 - 0.5f;
	mapCoords.y = (screenCoords.y / (_tileHeight / 2) - (screenCoords.x / (_tileWidth / 2))) / 2 + 0.5f;
	return sf::Vector2f(mapCoords);
}