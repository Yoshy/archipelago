#ifndef ASSET_REGISTRY_H
#define ASSET_REGISTRY_H

#include <memory>
#include <map>
#include <SFML/Graphics.hpp>
#include "natural_resources_specification.h"
#include "wares_specification.h"
#include "building_specification.h"

namespace Archipelago {

	typedef std::map<std::string, sf::Texture> texture_atlas_t;
	typedef std::map<WaresTypeId, Archipelago::WaresSpecification> wares_atlas_t;
	typedef std::map<NaturalResourceTypeId, Archipelago::NaturalResourceSpecification> natres_atlas_t;
	typedef std::map<BuildingTypeId, Archipelago::BuildingSpecification> buildings_atlas_t;

	enum class AssetType { Texture, NaturalResource, Ware, Building };

	class AssetRegistry {
	public:
		void loadTexture(const std::string& assetName, const std::string& filename);
		//void loadMap(const std::string& assetName, const std::string& filename, World* world);
		sf::Texture* getTexture(const std::string& textureName);
		//Archipelago::Map& getMap(const std::string& mapName);
		void prepareWaresAtlas();
		void prepareNaturalResourcesAtlas();
		void prepareBuildingAtlas();
		const std::string& getWaresName(WaresTypeId type) { return _wareAtlas.at(type).name; };
		const WaresSpecification& getWaresSpecification(WaresTypeId type) { return _wareAtlas.at(type); }
		const NaturalResourceSpecification& getNatresSpecification(NaturalResourceTypeId type) { return _natresAtlas.at(type); }
		const BuildingSpecification& getBuildingSpecification(BuildingTypeId type) { return _buildingAtlas.at(type); }
	private:
		texture_atlas_t _textureAtlas;
		wares_atlas_t _wareAtlas;
		natres_atlas_t _natresAtlas;
		buildings_atlas_t _buildingAtlas;
	};
}

#endif // ASSET_REGISTRY_H
