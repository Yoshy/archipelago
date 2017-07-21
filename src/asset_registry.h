#ifndef ASSET_REGISTRY_H
#define ASSET_REGISTRY_H

#include <spdlog/spdlog.h>
#include <memory>
#include <map>
#include <SFML/Graphics.hpp>
#include "map.h"

namespace Archipelago {

	enum class AssetType { Texture, Map };

	class AssetRegistry {
	public:
		AssetRegistry(std::shared_ptr<spdlog::logger> logger);
		~AssetRegistry();
		void loadAssetFromFile(AssetType t, const char* assetName, const char* filename);
	private:
		void _loadTexture(const char* assetName, const char* filename);
		void _loadMap(const char* assetName, const char* filename);
		std::map<std::string, sf::Texture> _textures;
		std::map<std::string, Archipelago::Map> _maps;
		std::shared_ptr<spdlog::logger> _logger;
	};
}

#endif // ASSET_REGISTRY_H
