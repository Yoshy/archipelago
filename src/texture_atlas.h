#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include <memory>
#include <map>
#include <string>
#include "SFML/Graphics.hpp"

namespace Archipelago {
	class TextureAtlas {
	public:
		TextureAtlas();
		TextureAtlas(const TextureAtlas&) = delete;
		~TextureAtlas();
		bool loadFromFile(const std::string& textureName, const std::string& textureFileName);
		sf::Texture& getTexture(const std::string& textureName);
	private:
		std::map<std::string, sf::Texture> _textures;
	};
}

#endif // TEXTURE_ATLAS_H
