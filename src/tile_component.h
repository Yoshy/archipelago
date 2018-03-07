#pragma once

#include <cstdint>
#include <SFML/Graphics.hpp>

namespace Archipelago {

	/** Tile component
	* This component contains data for graphical representation of one tile on the map
	*/
	struct TileComponent {
		TileComponent() : name(""), rising(0), x(0), y(0) {};
		TileComponent(std::string _name, uint32_t _rising, sf::Sprite _sprite, unsigned int _x, unsigned int _y) : name(_name), rising(_rising), sprite(_sprite), x(_x), y(_y) {};
		std::string name;
		unsigned int x;
		unsigned int y;
		uint32_t rising;
		sf::Sprite sprite;
	};

} // namespace Archipelago
