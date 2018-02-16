#ifndef ASSET_REGISTRY_H
#define ASSET_REGISTRY_H

#include <memory>
#include <map>
#include <SFML/Graphics.hpp>
#include "wares.h"

namespace Archipelago {

	typedef std::map<std::string, sf::Texture> texture_atlas_t;
	typedef std::map<WaresTypeId, Archipelago::WaresSpecification> wares_atlas_t;

	enum class AssetType { Texture, Map, Ware };

	class AssetRegistry {
	public:
		void loadTexture(const std::string& assetName, const std::string& filename);
		//void loadMap(const std::string& assetName, const std::string& filename, World* world);
		sf::Texture* getTexture(const std::string& textureName);
		//Archipelago::Map& getMap(const std::string& mapName);
		void prepareWaresAtlas();
		const std::string& getWaresName(WaresTypeId type) { return _waresAtlas.at(type).name; };
		const WaresSpecification& getWaresSpecification(WaresTypeId type) { return _waresAtlas.at(type); }
	private:
		texture_atlas_t _textureAtlas;
		wares_atlas_t _waresAtlas; 
	};
}

#endif // ASSET_REGISTRY_H
