#ifndef ASSET_REGISTRY_H
#define ASSET_REGISTRY_H

#include <memory>
#include <map>
#include <SFML/Graphics.hpp>
#include "map.h"
#include "texture_atlas.h"

namespace Archipelago {

	enum class AssetType { Texture, Map };

	class AssetRegistry {
	public:
		AssetRegistry();
		~AssetRegistry();
		void loadAssetFromFile(AssetType t, const std::string& assetName, const std::string& filename);
		Archipelago::Map& getMap(const std::string& mapName);
	private:
		void _loadTexture(const std::string& assetName, const std::string& filename);
		void _loadMap(const std::string& assetName, const std::string& filename);
		TextureAtlas _textureAtlas;
		std::map<std::string, Archipelago::Map> _maps;
	};
}

#endif // ASSET_REGISTRY_H
