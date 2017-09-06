#ifndef ASSET_REGISTRY_H
#define ASSET_REGISTRY_H

#include <memory>
#include <map>
#include <SFML/Graphics.hpp>
#include "map.h"
#include "goods.h"

namespace Archipelago {

	typedef std::map<std::string, sf::Texture> texture_atlas_t;
	typedef std::map<std::string, Archipelago::Map> map_atlas_t;
	typedef std::map<GoodsType, Archipelago::GoodsSpecification> goods_atlas_t;

	enum class AssetType { Texture, Map, Resource };

	class AssetRegistry {
	public:
		void loadTexture(const std::string& assetName, const std::string& filename);
		void loadMap(const std::string& assetName, const std::string& filename);
		sf::Texture* getTexture(const std::string& textureName);
		Archipelago::Map& getMap(const std::string& mapName);
		void prepareGoodsAtlas();
		const std::string& getGoodsName(GoodsType type) { return _goodsAtlas.at(type).name; };
		const GoodsSpecification& getGoodsSpecification(GoodsType type) { return _goodsAtlas.at(type); }
	private:
		texture_atlas_t _textureAtlas;
		map_atlas_t _mapAtlas;
		goods_atlas_t _goodsAtlas; 
	};
}

#endif // ASSET_REGISTRY_H
