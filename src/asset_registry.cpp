#include <fstream>
#include <spdlog/spdlog.h>
#include <json.hpp>
#include <ECS.h>
#include "asset_registry.h"
#include "game.h"

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

sf::Texture* AssetRegistry::getTexture(const std::string& textureName) {
	auto m = _textureAtlas.find(textureName);
	if (m != _textureAtlas.end()) {
		return &(m->second);
	}
	std::string s("Texture '" + textureName + "' not found in registry");
	spdlog::get(loggerName)->error(s);
	return nullptr;
}

void AssetRegistry::prepareWaresAtlas() {
	spdlog::get(loggerName)->trace("AssetRegistry::prepareWaresAtlas started...");
	std::string filename("assets/wares_specification.json");
	nlohmann::json waresSpecJSON;
	std::fstream waresSpecFile;
	waresSpecFile.open(filename);
	if (waresSpecFile.fail()) {
		spdlog::get(loggerName)->error("AssetRegistry::prepareWaresAtlas failed. Error opening specification file '{}'", filename);
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
			_wareAtlas.insert(std::pair<WaresTypeId, Archipelago::WaresSpecification>(static_cast<WaresTypeId>(waresSpec.at("id").get<int>()), std::move(gs)));
			spdlog::get(loggerName)->trace("Wares Specification loaded: '{}'", (waresSpec.at("name")).get<std::string>());
		}
	}
	catch (std::out_of_range& e) {
		spdlog::get(loggerName)->error("AssetRegistry::prepareWaresAtlas: Can't parse wares specifications: {}", e.what());
		exit(-1);
	}
	spdlog::get(loggerName)->trace("Wares atlas contains {} wares specifications", _wareAtlas.size());
}

void AssetRegistry::prepareNaturalResourcesAtlas() {
	spdlog::get(loggerName)->trace("AssetRegistry::prepareNaturalResourcesAtlas started...");
	std::string filename("assets/natural_resources_specification.json");
	nlohmann::json natresSpecJSON;
	std::fstream natresSpecFile;
	natresSpecFile.open(filename);
	if (natresSpecFile.fail()) {
		spdlog::get(loggerName)->error("AssetRegistry::prepareNaturalResourcesAtlas failed. Error opening specification file '{}'", filename);
		return;
	}
	natresSpecFile >> natresSpecJSON;
	natresSpecFile.close();
	try {
		for (auto natresSpec : natresSpecJSON) {
			NaturalResourceSpecification nrs;
			nrs.name = std::move(natresSpec.at("name").get<std::string>());
			loadTexture(natresSpec.at("name"), natresSpec.at("icon"));
			nrs.icon = getTexture(natresSpec.at("name"));
			_natresAtlas.insert(std::pair<NaturalResourceTypeId, Archipelago::NaturalResourceSpecification>(static_cast<NaturalResourceTypeId>(natresSpec.at("id").get<int>()), std::move(nrs)));
			spdlog::get(loggerName)->trace("Natural resources specification loaded: '{}'", (natresSpec.at("name")).get<std::string>());
		}
	}
	catch (std::out_of_range& e) {
		spdlog::get(loggerName)->error("AssetRegistry::prepareNaturalResourcesAtlas: Can't parse natural resources specifications: {}", e.what());
		exit(-1);
	}
	spdlog::get(loggerName)->trace("Natural resources atlas contains {} natural resources specifications", _natresAtlas.size());
}

void AssetRegistry::prepareBuildingAtlas() {
	spdlog::get(loggerName)->trace("AssetRegistry::prepareBuildingAtlas started...");
	std::string filename("assets/buildings_specification.json");
	nlohmann::json buildingSpecJSON;
	std::fstream buildingSpecFile;
	buildingSpecFile.open(filename);
	if (buildingSpecFile.fail()) {
		spdlog::get(loggerName)->error("AssetRegistry::prepareBuildingAtlas failed. Error opening specification file '{}'", filename);
		return;
	}
	buildingSpecFile >> buildingSpecJSON;
	buildingSpecFile.close();
	try {
		for (auto buildingSpec : buildingSpecJSON) {
			BuildingSpecification bs;
			bs.name = std::move(buildingSpec.at("name").get<std::string>());
			bs.description = std::move(buildingSpec.at("description").get<std::string>());
			loadTexture(buildingSpec.at("name"), buildingSpec.at("icon"));
			bs.icon = getTexture(buildingSpec.at("name"));
			bs.tileRising = buildingSpec.at("tile_rising");
			bs.natresRequired = static_cast<NaturalResourceTypeId>(buildingSpec.at("natres_required").get<int>());
			for (auto wareReq : buildingSpec.at("wares_required")) {
				WaresStack ws;
				ws.type = static_cast<WaresTypeId>(wareReq.at("type").get<int>());
				ws.amount = wareReq.at("amount").get<int>();
				bs.waresRequired.push_back(ws);
			}
			std::vector<int> briv = std::move(buildingSpec.at("building_required").get<std::vector<int>>());
			for (auto br : briv) {
				bs.buildingsRequired.push_back(static_cast<BuildingTypeId>(br));
			}
			for (auto providedInstantWare : buildingSpec.at("provided_instant_wares")) {
				WaresStack ws;
				ws.type = static_cast<WaresTypeId>(providedInstantWare.at("type").get<int>());
				ws.amount = providedInstantWare.at("amount").get<int>();
				bs.providedInstantWares.push_back(ws);
			}
			for (auto wareProd : buildingSpec.at("wares_produced")) {
				WaresStack ws;
				ws.type = static_cast<WaresTypeId>(wareProd.at("type").get<int>());
				ws.amount = wareProd.at("amount").get<int>();
				bs.waresProduced.push_back(ws);
			}
			_buildingAtlas.insert(std::pair<BuildingTypeId, Archipelago::BuildingSpecification>(static_cast<BuildingTypeId>(buildingSpec.at("id").get<int>()), std::move(bs)));
			spdlog::get(loggerName)->trace("Building specification loaded: '{}'", (buildingSpec.at("name")).get<std::string>());
		}
	}
	catch (std::out_of_range& e) {
		spdlog::get(loggerName)->error("AssetRegistry::prepareBuildingAtlas: Can't parse building specifications: {}", e.what());
		exit(-1);
	}
	spdlog::get(loggerName)->trace("Natural resources atlas contains {} building specifications", _natresAtlas.size());
}