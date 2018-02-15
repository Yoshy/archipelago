#pragma once

#include <cstdint>
#include <SFML/Graphics.hpp>

namespace Archipelago {

	/** Tile component
	* This component contains data for graphical representation of one tile on the map
	*/
	struct TileComponent {
		TileComponent() : _rising(0) {};
		TileComponent(uint32_t rising, sf::Sprite sprite) : _rising(rising), _sprite(sprite) {};
		uint32_t _rising;
		sf::Sprite _sprite;
	};

} // namespace Archipelago
