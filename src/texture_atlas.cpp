#include "globals.h"
#include "texture_atlas.h"
#include "spdlog/spdlog.h"

using namespace Archipelago;

TextureAtlas::TextureAtlas() {
}

TextureAtlas::~TextureAtlas() {
}

bool TextureAtlas::loadFromFile(const std::string& textureName, const std::string& textureFileName) {
	sf::Texture texture;
	if (!texture.loadFromFile(textureFileName)) {
		spdlog::get(LOGGER_NAME)->trace("Error loading texture '{}' from file '{}'", textureName, textureFileName);
		return false;
	};
	_textures[textureName] = texture;
	spdlog::get(LOGGER_NAME)->trace("loaded texture '{}' from file '{}', size {}x{}", textureName, textureFileName, texture.getSize().x, texture.getSize().y);
	return true;
}

sf::Texture& TextureAtlas::getTexture(const std::string& textureName) {
	return _textures[textureName];
}
