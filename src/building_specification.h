#pragma once

#include <string>
#include "natural_resources_specification.h"
#include "wares_specification.h"

class sf::Texture;

namespace Archipelago {

	enum class BuildingTypeId { Unknown = 0, BaseCamp = 1, Woodcutter = 2, _Last = Woodcutter, _First = BaseCamp };

	struct BuildingSpecification {
		std::string name;
		std::string description;
		sf::Texture* icon; // non-owning pointer
		unsigned int tileRising;
		NaturalResourceTypeId natresRequired;
		std::vector<WaresStack> waresRequired;
		std::vector<BuildingTypeId> buildingRequired;
		WaresTypeId productionType;
		unsigned int productionAmountPerMonth;
	};

}
