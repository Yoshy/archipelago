#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include "asset_registry.h"
#include "game.h"
#include "spdlog/logger.h"

using namespace Archipelago;
using namespace std;
using namespace rapidjson;

AssetRegistry::AssetRegistry(std::shared_ptr<spdlog::logger> logger) {
	_logger = logger;
}

AssetRegistry::~AssetRegistry() {
}

void AssetRegistry::loadAssetFromFile(AssetType assetType, const char* assetName, const char* filename) {
	std::fstream assetFile;

	switch (assetType) {
	case AssetType::Texture: // load image as texture
		_loadTexture(assetName, filename);
		break;
	case AssetType::Map: // load map
		_loadMap(assetName, filename);
		break;
	}
}

void AssetRegistry::_loadTexture(const char* assetName, const char* filename) {
	sf::Texture texture;
	texture.loadFromFile(filename);
	_textures[assetName] = texture;
}

void AssetRegistry::_loadMap(const char* assetName, const char* filename) {
	Archipelago::Map map;
	fstream mapFile;
	mapFile.open(filename);
	string strimap, sLine;
	while (mapFile >> sLine) {
		strimap += sLine;
	}
	mapFile.close();
	Document mapDOM;
	ParseResult pr = mapDOM.Parse<kParseCommentsFlag>(strimap.c_str());
	if (!pr) {
		_logger->error("Error while parsing configuration file: '{}', offset: {}", GetParseError_En(pr.Code()), pr.Offset());
	}

}