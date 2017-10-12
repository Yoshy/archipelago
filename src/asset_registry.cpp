#include <fstream>
#include "asset_registry.h"
#include "game.h"
#include "spdlog/spdlog.h"
#include "json.hpp"

using namespace Archipelago;

void AssetRegistry::loadTexture(const std::string& assetName, const std::string& filename) {
	sf::Texture texture;
	if (!texture.loadFromFile(filename)) {
		spdlog::get(loggerName)->trace("Error loading texture '{}' from file '{}'", assetName, filename);
		return;
	};
	_textureAtlas.insert(std::pair<std::string, sf::Texture>(assetName, std::move(texture)));
	spdlog::get(loggerName)->trace("Loaded texture '{}' from file '{}', size {}x{}", assetName, filename, texture.getSize().x, texture.getSize().y);
	spdlog::get(loggerName)->trace("Texture atlas contains {} textures", _textureAtlas.size());
}

void AssetRegistry::loadMap(const std::string& assetName, const std::string& filename) {
	Archipelago::Map map(*this);
	map.loadFromFile(filename);
	_mapAtlas.insert(std::pair<std::string, Archipelago::Map>(assetName, std::move(map)));
	spdlog::get(loggerName)->trace("Loaded map '{}' from file '{}'", assetName, filename);
}

sf::Texture* AssetRegistry::getTexture(const std::string& textureName) {
	auto m = _textureAtlas.find(textureName);
	if (m != _textureAtlas.end()) {
		return &(m->second);
	}
	std::string s("Texture '" + textureName + "' not found in registry");
	spdlog::get(loggerName)->error(s);
	return nullptr;
}

Archipelago::Map& AssetRegistry::getMap(const std::string& mapName) {
	auto m = _mapAtlas.find(mapName);
	if (m != _mapAtlas.end()) {
		return m->second;
	}
	std::string s("Map '" + mapName + "' not found in registry");
	spdlog::get(loggerName)->error(s);
	throw std::out_of_range(s);
}

void AssetRegistry::prepareWaresAtlas() {
	spdlog::get(loggerName)->trace("AssetRegistry::prepareWaresAtlas started...");
	std::string filename("assets/wares_specification.json");
	nlohmann::json waresSpecJSON;
	std::fstream waresSpecFile;
	waresSpecFile.open(filename);
	if (waresSpecFile.fail()) {
		spdlog::get(loggerName)->error("AssetRegistry::prepareWaresAtlas failed. Error opening wares specification file '{}'", filename);
		return;
	}
	waresSpecFile >> waresSpecJSON;
	waresSpecFile.close();
	try {
		for (auto waresSpec : waresSpecJSON) {
			WaresSpecification gs;
			gs.name = std::move(waresSpec.at("name").get<std::string>());
			loadTexture(waresSpec.at("name"), waresSpec.at("icon"));
			gs.icon = getTexture(waresSpec.at("name"));
			_waresAtlas.insert(std::pair<WaresTypeId, Archipelago::WaresSpecification>(static_cast<WaresTypeId>(waresSpec.at("id").get<int>()), std::move(gs)));
			spdlog::get(loggerName)->trace("Wares Specification loaded: '{}'", (waresSpec.at("name")).get<std::string>());
		}
	}
	catch (std::out_of_range& e) {
		spdlog::get(loggerName)->error("AssetRegistry::prepareWaresAtlas: Can't parse wares specifications: {}", e.what());
		exit(-1);
	}
	spdlog::get(loggerName)->trace("Wares atlas contains {} wares specifications", _waresAtlas.size());
}
