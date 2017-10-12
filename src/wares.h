#ifndef RESOURCE_H
#define RESOURCE_H

#include "SFML/Graphics.hpp"
#include <string>
#include <memory>

namespace Archipelago {

	enum class WaresTypeId { Unknown = 0, People = 1, FreshWater = 2, SaltWater = 3, Crops = 4, Wood = 5, _End, _Begin = People };

	struct WaresSpecification {
		std::string name;
		sf::Texture* icon;
	};

	struct WaresStack {
		WaresTypeId type;
		int amount;
	};

}

#endif RESOURCE_H