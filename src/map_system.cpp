#include <spdlog\spdlog.h>
#include <fstream>
#include <json.hpp>
#include "map_system.h"

using namespace Archipelago;

void MapSystem::configure(World* world) {
	spdlog::get(loggerName)->trace("MapSystem::configure started");
	world->subscribe<LoadMapEvent>(this);
	world->subscribe<MouseMovedEvent>(this);
	world->subscribe<MoveCameraEvent>(this);
	world->subscribe<MoveCameraToMapCenterEvent>(this);
	world->subscribe<ConvertScreenToMapCoordsEvent>(this);
	world->subscribe<ConvertMapToScreenCoordsEvent>(this);
	world->subscribe<ShowNaturalResourcesEvent>(this);
	world->subscribe<RequestHighlightedEntityEvent>(this);
	world->subscribe<RenderMapEvent>(this);
	_showNaturalResources = false;
	_currentHighlightedEntity = 0;
}

void MapSystem::unconfigure(World* world) {
	spdlog::get(loggerName)->trace("MapSystem::unconfigure started");
	world->unsubscribe<LoadMapEvent>(this);
	world->unsubscribe<MouseMovedEvent>(this);
	world->unsubscribe<MoveCameraEvent>(this);
	world->unsubscribe<MoveCameraToMapCenterEvent>(this);
	world->unsubscribe<ConvertScreenToMapCoordsEvent>(this);
	world->unsubscribe<ConvertMapToScreenCoordsEvent>(this);
	world->unsubscribe<ShowNaturalResourcesEvent>(this);
	world->unsubscribe<RequestHighlightedEntityEvent>(this);
	world->unsubscribe<RenderMapEvent>(this);
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

	unsigned int mapSize = 0;
	std::vector<unsigned int> terrainLayer;
	std::vector<unsigned int> natresLayer;
	try {
		_mapWidth = mapJSON.at("mapWidth");
		_mapHeight = mapJSON.at("mapHeight");
		_tileWidth = mapJSON.at("tileWidth");
		_tileHeight = mapJSON.at("tileHeight");
		mapSize = _mapWidth * _mapHeight;
		terrainLayer = std::move(mapJSON.at("terrain_layer").get<std::vector<unsigned int>>());
		natresLayer = std::move(mapJSON.at("resources_layer").get<std::vector<unsigned int>>());
		if (terrainLayer.size() != mapSize) {
			logger->error("terrain_layer size ({}) is not equal to width*height ({}). There is something wrong in map file.", terrainLayer.size(), mapSize);
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
		tileComponent.sprite.setTexture(*_game.getAssetRegistry().getTexture(tileName), true);
		tileComponent.rising = tileRising;
	}
	unsigned int mapX = 0, mapY = 0;
	TileComponent tmpTileComponent;
	for (unsigned int i = 0; i < mapSize; i++) {
		Entity* ent = world->create();
		tmpTileComponent = tileAtlas.at(terrainLayer[i]);
		NaturalResourceTypeId natresType;
		uint32_t bitMask = 0xFF000000;
		unsigned int shift = 24;
		for (unsigned int waresIdx = 0; waresIdx < 4; waresIdx++) {
			natresType = static_cast<NaturalResourceTypeId>((natresLayer[i] & bitMask) >> shift);
			shift -= 8;
			bitMask = bitMask >> 8;
			if (natresType != NaturalResourceTypeId::Unknown) {
				ent->assign<NaturalResourceComponent>(natresLayer[i]);
			};
		}
		sf::Vector2f screenCoords = _mapToScreenCoords(sf::Vector2f(static_cast<float>(mapX), static_cast<float>(mapY)));
		screenCoords.y = screenCoords.y - tmpTileComponent.rising;
		tmpTileComponent.sprite.setPosition(screenCoords);
		ent->assign<TileComponent>(tmpTileComponent.rising, tmpTileComponent.sprite, mapX, mapY);
		ent->assign<NaturalResourceComponent>(natresLayer[i]);
		mapX++;
		if (mapX >= _mapWidth) {
			mapX = 0;
			mapY++;
		}
	}
	logger->trace("MapSystem: Map loaded. Width: {}, height: {}", _mapWidth, _mapHeight);
}

void MapSystem::receive(World* world, const MouseMovedEvent& event) {
	//// Highlight tile, if it lies under mouse cursor
	//sf::Vector2f mouseScreenCoords = _game.getRenderWindow().mapPixelToCoords(sf::Mouse::getPosition(_game.getRenderWindow()));
	//sf::Vector2f mouseMapCoords = _screenToMapCoords(mouseScreenCoords);
	//if (_currentHighlightedEntity) _currentHighlightedEntity->get<TileComponent>()->sprite.setColor(sf::Color::White);

	//// FIXME: Ugly stuff. FPS killing code...
	//world->each<TileComponent>([&](Entity* ent, ComponentHandle<TileComponent> tile) {
	//	if (static_cast<int>(mouseMapCoords.x) == tile->x && static_cast<int>(mouseMapCoords.y) == tile->y) {
	//		tile->sprite.setColor(sf::Color(255, 255, 255, 127));
	//		_currentHighlightedEntity = ent;
	//	}
	//});
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

void MapSystem::receive(World* world, const ConvertScreenToMapCoordsEvent& event) {
	event.coords = _screenToMapCoords(event.coords);
	if (event.coords.x > _mapWidth) event.coords.x = -1;
	if (event.coords.y > _mapHeight) event.coords.y = -1;
}

void MapSystem::receive(World* world, const ConvertMapToScreenCoordsEvent& event) {
	event.coords = _mapToScreenCoords(event.coords);
}

void MapSystem::receive(World* world, const ShowNaturalResourcesEvent& event) {
	_showNaturalResources = event.show;
}

void MapSystem::receive(World* world, const RequestHighlightedEntityEvent& event) {
	event.entityID = _currentHighlightedEntity;
}

void MapSystem::receive(World* world, const RenderMapEvent& event) {
	sf::Vector2f mouseScreenCoords = _game.getRenderWindow().mapPixelToCoords(sf::Mouse::getPosition(_game.getRenderWindow()));
	sf::Vector2f mouseMapCoords = _screenToMapCoords(mouseScreenCoords);
	//if (_currentHighlightedEntity) world->getById(_currentHighlightedEntity)->get<TileComponent>()->sprite.setColor(sf::Color::White);
	_currentHighlightedEntity = 0;
	world->each<TileComponent>([&](Entity* ent, ComponentHandle<TileComponent> tile) {
		if (static_cast<int>(mouseMapCoords.x) == tile->x && static_cast<int>(mouseMapCoords.y) == tile->y) {
			//tile->sprite.setColor(sf::Color(255, 255, 255, 127));
			_currentHighlightedEntity = ent->getEntityId();
		}
		// Draw tile
		_game.getRenderWindow().draw(tile->sprite);
		// Draw natural resources on tile
		if (_showNaturalResources) {
			uint32_t resourceSet = ent->get<NaturalResourceComponent>()->resourceSet;
			uint32_t mask = 0x000000FF;
			// Формат хранения натуральных ресурсов: 32-битное число, каждый байт обозначает доступный на тайле тип ресурса, т.о.
			// на одном тайле может быть доступно до четырёх типов ресурсов.
			// На тайл влезет примерно (tileWidth - iconWidth) * 2 иконок
			sf::Sprite natresSprite;
			for (unsigned int g = 0; g < 4; g++) {
				int numWares = 1;
				NaturalResourceTypeId natresType = static_cast<NaturalResourceTypeId>((resourceSet & mask) >> (g*8));
				mask = mask << 8;
				if ((natresType >= NaturalResourceTypeId::_First) && (natresType <= NaturalResourceTypeId::_Last)) {
					natresSprite.setTexture(*_game.getAssetRegistry().getNatresSpecification(natresType).icon, true);
					auto gsTexSize = natresSprite.getTexture()->getSize();
					auto natresSpritePos = tile->sprite.getPosition();
					natresSpritePos.x += (_tileWidth / 2) + ((gsTexSize.x) * (g - (numWares / 2)));
					natresSpritePos.y += (_tileHeight / 2) - (gsTexSize.y / 2);
					natresSprite.setPosition(natresSpritePos);
					_game.getRenderWindow().draw(natresSprite);
				}
			}
		}
	});
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

int MapSystem::_numberOfSetBits(uint32_t value) {
	value = value - ((value >> 1) & 0x55555555);
	value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
	return (((value + (value >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}