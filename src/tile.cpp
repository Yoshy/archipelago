#include "tile.h"
#include <memory>
#include "spdlog/spdlog.h"
#include "asset_registry.h"


using namespace Archipelago;

Tile::Tile(AssetRegistry& assets) : _rising(0), _name(""), _assets(assets) {

}

const std::string& Tile::getName() {
	return _name;
}

void Tile::setName(const std::string& tileName) {
	_name = tileName;
}

void Tile::setTexture(const sf::Texture& texture) {
	_terrain_sprite.setTexture(texture);
}

void Tile::setTileRising(unsigned int rising) {
	_rising = rising;
}

unsigned int Tile::getTileRising() {
	return _rising;
}

sf::Sprite& Tile::getSprite() {
	return _terrain_sprite;
}

void Tile::setSpritePosition(float x, float y) {
	_terrain_sprite.setPosition(x, y);
}

void Tile::setSpritePosition(sf::Vector2f pos) {
	_terrain_sprite.setPosition(pos);
}

void Tile::addGoods(GoodsTypeId type, int amount) {
	_goodsStackList.push_back({type, amount});
}
