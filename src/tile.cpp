#include "tile.h"

using namespace Archipelago;

Tile::Tile() : _rising(0), _name("") {

}

Tile::~Tile() {

}

const std::string& Tile::getName() {
	return _name;
}

void Tile::setName(const std::string& tileName) {
	_name = tileName;
}

void Tile::setTexture(const sf::Texture& texture) {
	_sprite.setTexture(texture);
}

void Tile::setTileRising(unsigned int baseHeight) {
	_rising = baseHeight;
}

unsigned int Tile::getBaseHeight() {
	return _rising;
}

sf::Sprite& Tile::getSprite() {
	return _sprite;
}

void Tile::setSpritePosition(float x, float y) {
	_sprite.setPosition(x, y);
}

void Tile::setSpritePosition(sf::Vector2f pos) {
	_sprite.setPosition(pos);
}