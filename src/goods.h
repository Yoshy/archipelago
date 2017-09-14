#ifndef RESOURCE_H
#define RESOURCE_H

#include "SFML/Graphics.hpp"
#include <string>
#include <memory>

namespace Archipelago {

	enum class GoodsTypeId { Unknown = 0, FreshWater = 1, SaltWater = 2, Crops = 3, Wood = 4 };

	struct GoodsSpecification {
		std::string name;
		sf::Texture* icon;
	};

	struct GoodsStack {
		GoodsTypeId type;
		int amount;
	};

}

#endif RESOURCE_H