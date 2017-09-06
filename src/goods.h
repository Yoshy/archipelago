#ifndef RESOURCE_H
#define RESOURCE_H

#include "SFML/Graphics.hpp"
#include <string>
#include <memory>

namespace Archipelago {

	enum class GoodsType { Unknown = 0, FreshWater = 1, SaltWater = 2, Wheat = 3 };

	struct GoodsSpecification {
		std::string name;
		sf::Texture* icon;
	};

	struct GoodsStack {
		GoodsType type;
		int amount;
	};

}

#endif RESOURCE_H