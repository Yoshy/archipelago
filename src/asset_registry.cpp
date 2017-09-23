#include <fstream>
#include "asset_registry.h"
#include "game.h"
#include "globals.h"
#include "spdlog/spdlog.h"
#include "json.hpp"

using namespace Archipelago;

void AssetRegistry::loadTexture(const std::string& assetName, const std::string& filename) {
	sf::Texture texture;
	if (!texture.loadFromFile(filename)) {
		spdlog::get(LOGGER_NAME)->trace("Error loading texture '{}' from file '{}'", assetName, filename);
		return;
	};
	_textureAtlas.insert(std::pair<std::string, sf::Texture>(assetName, std::move(texture)));
	spdlog::get(LOGGER_NAME)->trace("Loaded texture '{}' from file '{}', size {}x{}", assetName, filename, texture.getSize().x, texture.getSize().y);
	spdlog::get(LOGGER_NAME)->trace("Texture atlas contains {} textures", _textureAtlas.size());
}

void AssetRegistry::loadMap(const std::string& assetName, const std::string& filename) {
	Archipelago::Map map(*this);
	map.loadFromFile(filename);
	_mapAtlas.insert(std::pair<std::string, Archipelago::Map>(assetName, std::move(map)));
	spdlog::get(LOGGER_NAME)->trace("Loaded map '{}' from file '{}'", assetName, filename);
}

sf::Texture* AssetRegistry::getTexture(const std::string& textureName) {
	auto m = _textureAtlas.find(textureName);
	if (m != _textureAtlas.end()) {
		return &(m->second);
	}
	std::string s("Texture '" + textureName + "' not found in registry");
	spdlog::get(LOGGER_NAME)->error(s);
	return nullptr;
}

Archipelago::Map& AssetRegistry::getMap(const std::string& mapName) {
	auto m = _mapAtlas.find(mapName);
	if (m != _mapAtlas.end()) {
		return m->second;
	}
	std::string s("Map '" + mapName + "' not found in registry");
	spdlog::get(LOGGER_NAME)->error(s);
	throw std::out_of_range(s);
}

void AssetRegistry::prepareGoodsAtlas() {
	spdlog::get(LOGGER_NAME)->trace("AssetRegistry::prepareGoodsAtlas started...");
	std::string filename("assets/goods_specification.json");
	nlohmann::json goodsSpecJSON;
	std::fstream goodsSpecFile;
	goodsSpecFile.open(filename);
	if (goodsSpecFile.fail()) {
		spdlog::get(LOGGER_NAME)->error("AssetRegistry::prepareGoodsAtlas failed. Error opening goods specification file '{}'", filename);
		return;
	}
	goodsSpecFile >> goodsSpecJSON;
	goodsSpecFile.close();
	try {
		for (auto goodsSpec : goodsSpecJSON) {
			GoodsSpecification gs;
			gs.name = std::move(goodsSpec.at("name").get<std::string>());
			loadTexture(goodsSpec.at("name"), goodsSpec.at("icon"));
			gs.icon = getTexture(goodsSpec.at("name"));
			_goodsAtlas.insert(std::pair<GoodsTypeId, Archipelago::GoodsSpecification>(static_cast<GoodsTypeId>(goodsSpec.at("id").get<int>()), std::move(gs)));
			spdlog::get(LOGGER_NAME)->trace("Goods Specification loaded: '{}'", (goodsSpec.at("name")).get<std::string>());
		}
	}
	catch (std::out_of_range& e) {
		spdlog::get(LOGGER_NAME)->error("AssetRegistry::prepareGoodsAtlas: Can't parse goods specifications: {}", e.what());
		exit(-1);
	}
	spdlog::get(LOGGER_NAME)->trace("Goods atlas contains {} goods specifications", _goodsAtlas.size());
}
