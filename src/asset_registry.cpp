#include "asset_registry.h"
#include "game.h"

using namespace Archipelago;

AssetRegistry::AssetRegistry() {
}

AssetRegistry::~AssetRegistry() {
}

void AssetRegistry::loadAssetFromFile(AssetType assetType, const std::string& assetName, const std::string& filename) {
	switch (assetType) {
	case AssetType::Texture: // load image as texture
		_loadTexture(assetName, filename);
		break;
	case AssetType::Map: // load map
		_loadMap(assetName, filename);
		break;
	}
}

Archipelago::Map& AssetRegistry::getMap(const std::string& mapName) {
	if (_maps.find(mapName) != _maps.end()) {
		return _maps[mapName];
	}
	throw std::out_of_range("Map '" + mapName + "' not found in registry");
}

void AssetRegistry::_loadTexture(const std::string& assetName, const std::string& filename) {
	_textureAtlas.loadFromFile(assetName, filename);
}

void AssetRegistry::_loadMap(const std::string& assetName, const std::string& filename) {
	Archipelago::Map map;
	map.setTextureAtlas(&_textureAtlas);
	map.loadFromFile(filename);
	_maps[assetName] = map;
}
